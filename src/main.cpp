#include "sdk.h"
//
#include <iostream>
#include <thread>
#include <memory>

#include <boost/program_options.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>

#include "json_loader.h"
#include "request_handler.h"
#include "logging_request_handler.h"
#include "application.h"

using namespace std::literals;
namespace net = boost::asio;    
using tcp = net::ip::tcp;

struct Args {
    std::string static_dir;
    std::string config;
    int tick_delta = 0;
    bool is_player_pos_random = false;
}; 

[[nodiscard]] std::optional<Args> ParseCommandLine(int argc, const char* const argv[]) {
    namespace po = boost::program_options;

    po::options_description desc{"All options"s};
    Args args;
    desc.add_options()           //
        ("help,h", "Show help")  //
        ("www-root,w", po::value(&args.static_dir)->value_name("dir"s), "Set static files root")  //
        ("config-file,c", po::value(&args.config)->value_name("file"s), "Set config file path") //
        ("tick-period,t", po::value(&args.tick_delta)->value_name("milliseconds"s), "Set tick period") //
        ("randomize-spawn-points,rnd", "Spawn dogs at random positions");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.contains("help"s)) {
        std::cout << desc;
        return std::nullopt;
    }

    if (!vm.contains("config-file"s)) {
        throw std::runtime_error("Config file is not been specified. See options (-h --help)"s);
    }

    if (!vm.contains("www-root"s)) {
        throw std::runtime_error("Static files dir is not specified. See options (-h --help)"s);
    }

    if (vm.contains("randomize-spawn-points"s)) {
        args.is_player_pos_random = true;
    }

    if (vm.contains("tick-period"s)) {
        if (args.tick_delta < 0) {
            args.tick_delta = 0;
        }
    }

    return args;
} 

namespace {

// Запускает функцию fn на n потоках, включая текущий
template <typename Fn>
void RunWorkers(unsigned n, const Fn& fn) {
    n = std::max(1u, n);
    std::vector<std::jthread> workers;
    workers.reserve(n - 1);
    // Запускаем n-1 рабочих потоков, выполняющих функцию fn
    while (--n) {
        workers.emplace_back(fn);
    }
    fn();
}

}  // namespace



int main(int argc, const char* argv[]) {
    using RequestHandler = http_handler::RequestHandler;
    using ApiHandler = api_handler::ApiHandler;
    using Application = app::Application;
    logging::add_common_attributes();

    logging::add_console_log( 
        std::clog,
        keywords::format = &MyFormatter
    );
    try {
        if (auto args = ParseCommandLine(argc, argv)) {
            // инициируем логгер
            

            // 1. Загружаем карту из файла и построить модель игры
            auto static_root_path = args.value().static_dir;
            // 2. Инициализируем io_context
            const unsigned num_threads = std::thread::hardware_concurrency();
            net::io_context ioc(num_threads);

            // 3. Добавляем асинхронный обработчик сигналов SIGINT и SIGTERM
            net::signal_set signals(ioc, SIGINT, SIGTERM);
            signals.async_wait([&ioc](const boost::system::error_code& ec, [[maybe_unused]] int signal_number) {
                if (!ec) {
                    ioc.stop();
                }
            });

            // 4. Создаём обработчик HTTP-запросов и связываем его с моделью игры
            net::strand<net::io_context::executor_type> strand{net::make_strand(ioc)};

            auto app = new Application(
                args.value().config, 
                args.value().tick_delta, 
                args.value().is_player_pos_random, 
                strand);

            auto handler = std::make_shared<RequestHandler>(*app, static_root_path);
            server_logging::LoggingRequestHandler<std::shared_ptr<RequestHandler>> log_handler(handler);

            // 5. Запустить обработчик HTTP-запросов, делегируя их обработчику запросов
            auto address = net::ip::make_address("0.0.0.0");
            constexpr net::ip::port_type port = 8080;

            tcp::endpoint ep(address, port);

            http_server::ServeHttp(ioc, ep, [&log_handler](tcp::endpoint& ep, auto&& req, auto&& send) {
                log_handler(ep, std::forward<decltype(req)>(req), std::forward<decltype(send)>(send));
            });

            // Эта надпись сообщает тестам о том, что сервер запущен и готов обрабатывать запросы
            json_loader::json::value data{
                {"port", port},
                {"address", address.to_string()}
            };

            BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, data) << "Server has started...";

            // // 6. Запускаем обработку асинхронных операций
            RunWorkers(std::max(1u, num_threads), [&ioc] {
                ioc.run();
            });    
        }
        return EXIT_SUCCESS;
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    }
        
}

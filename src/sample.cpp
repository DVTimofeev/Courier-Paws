#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>
#include <optional>
#include <vector>
#include <chrono>

using namespace std::literals; 

struct Args {
    std::string static_dir;
    std::string config;
    int tick_delta;
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
        ("randomize-spawn-points,t", "Spawn dogs at random positions");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.contains("help"s)) {
        std::cout << desc;
        return std::nullopt;
    }

    if (!vm.contains("config-file"s)) {
        throw std::runtime_error("Config file is not been specified"s);
    }

    if (!vm.contains("www-root"s)) {
        throw std::runtime_error("Static files dir is not specified"s);
    }
    return args;
} 

int main(int argc, char* argv[]) {
    try {
        if (auto args = ParseCommandLine(argc, argv)) {
            std::cout << "i'm HERE" << std::endl;
            std::cout << args.value().config << std::endl;
            std::cout << args.value().static_dir << std::endl;
        }
        return EXIT_SUCCESS;
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    }
} 
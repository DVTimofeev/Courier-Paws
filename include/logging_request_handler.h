#pragma once
#include <chrono>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

#include "logging.h"

namespace server_logging{

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;
namespace json = boost::json;
// Запрос, тело которого представлено в виде строки
using Request = http::request<http::string_body>;

// Ответ, тело которого представлено в виде строки
using Response = http::response<http::string_body>;

static std::string GetTimestamp() {
        // get a precise timestamp as a string
        const auto now = std::chrono::system_clock::now();
        const auto nowAsTimeT = std::chrono::system_clock::to_time_t(now);
        const auto nowMs = std::chrono::duration_cast<std::chrono::microseconds>(
            now.time_since_epoch()) % 1000000;
        std::stringstream nowSs;
        nowSs
            << std::put_time(std::localtime(&nowAsTimeT), "%FT%T")
            << '.' << std::setfill('0') << std::setw(6) << nowMs.count();
        return nowSs.str();
    }

template<class RequestHandler>
class LoggingRequestHandler {
public:
    LoggingRequestHandler(RequestHandler handler)
        : decorated_(std::move(handler)){
    }
    
    template <typename Body, typename Allocator, typename Send>
    void operator ()(tcp::endpoint ep, http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send){
        auto start_ts = Clock::now();

        LogRequest(ep, req);

        auto send_wrapper = [send = std::forward<Send>(send), start_ts, ep](auto&& response){
            auto end_ts = Clock::now();
            LogResponse(ep, response, end_ts - start_ts);
            send(std::forward<decltype(response)>(response));
        };

        (*decorated_)(ep, std::forward<decltype(req)>(req), send_wrapper);
        // decorated_(std::forward<decltype(req)>(req), send_wrapper);
    }



private:
    using Clock = std::chrono::system_clock;

    template <typename Body, typename Allocator>
    static void LogRequest( const tcp::endpoint& ep, 
                            const http::request<Body, 
                            http::basic_fields<Allocator>>& req){
        json::value data{
            {"ip", ep.address().to_string()},
            {"URI", req.target()},
            {"method", req.method_string()}
        };

        BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, data) << "request received";

    }

    template <class T>
    static void LogResponse(const tcp::endpoint& ep,
                            const http::response<T>& res,
                            Clock::duration dur){
        json::value data{
            {"ip", ep.address().to_string()},
            {"response_time", dur.count() / 100000},
            {"code", res.result_int()},
            {"content_type", res[http::field::content_type]}
        };
        BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, data) << "response sent";
    }
    RequestHandler decorated_;
};
} // namespace server_logging
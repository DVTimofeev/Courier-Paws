#pragma once
#include <iostream>

#include "http_server.h"
#include "application.h"
#include "api_handler.h"


namespace http_handler {
namespace net = boost::asio;
namespace fs = std::filesystem;
namespace beast = boost::beast;
namespace http = beast::http;
using tcp = boost::asio::ip::tcp;
using namespace std::literals;
namespace json = boost::json;

// ApiHandler
using ApiHandler = api_handler::ApiHandler;
using Game = model::Game;
using Application = app::Application;

// Запрос, тело которого представлено в виде строки
using StringRequest = http::request<http::string_body>;
// Ответ, тело которого представлено в виде строки
using StringResponse = http::response<http::string_body>;

// Ответ, тело которого представлено в виде файла
using FileResponse = http::response<http::file_body>;

struct ContentType {
    ContentType() = delete;
    constexpr static std::string_view TEXT_HTML = "text/html"sv;
    constexpr static std::string_view TEXT_CSS = "text/css"sv;
    constexpr static std::string_view TEXT_PLAIN = "text/plain"sv;
    constexpr static std::string_view TEXT_JS = "text/javascript"sv;
    constexpr static std::string_view APP_JSON = "application/json"sv;
    constexpr static std::string_view APP_XML = "application/xml"sv;
    constexpr static std::string_view APP_OCTET_STREAM = "application/octet-stream"sv;
    constexpr static std::string_view IMAGE_PNG = "image/png"sv;
    constexpr static std::string_view IMAGE_JPEG = "image/jpeg"sv;
    constexpr static std::string_view IMAGE_GIF = "image/gif"sv;
    constexpr static std::string_view IMAGE_BMP = "image/bmp"sv;
    constexpr static std::string_view IMAGE_ICO = "image/vnd.microsoft.icon"sv;
    constexpr static std::string_view IMAGE_TIFF = "image/tiff"sv;
    constexpr static std::string_view IMAGE_SVG_XML = "image/svg+xml"sv;
    constexpr static std::string_view AUDIO_MPEG = "audio/mpeg"sv;
};

class RequestHandler : public std::enable_shared_from_this<RequestHandler>{
public:
    using Strand = net::strand<net::io_context::executor_type>;

    // RequestHandler(ApiHandler& api_handler, const fs::path& root_path, Strand strand);
    RequestHandler(Application& app, const fs::path& root_path);
       
    RequestHandler(const RequestHandler&) = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;

    template <typename Body, typename Allocator, typename Send>
    void operator()(const tcp::endpoint& ep, http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
        auto version = req.version();
        auto keep_alive = req.keep_alive();
        try {
            if (api_handler_.IsApiRequest(req)){
                auto handle = [self = shared_from_this(), send, req = std::forward<decltype(req)>(req), version, keep_alive]{
                    try {
                        assert(self->api_strand_.running_in_this_thread());
                        return send(self->api_handler_.HandleRequest(req));
                    } catch (...){
                        send(self->ReportServerError(version, keep_alive));
                    }
                };
                return net::dispatch(api_strand_, handle);
            } else {
                send(MakeFileResponse(req));
            }
        } catch (...) {
            send(ReportServerError(version, keep_alive));
        }
    }

private:
    std::vector<std::string> ParseTarget(std::string_view target) const;
    
    StringResponse ReportServerError(unsigned version, bool keep_alive) const;
    FileResponse MakeFileResponse(const StringRequest& req) const;

    std::string_view GetContentType(std::string_view file_extention) const;

    fs::path root_path_;
    Strand& api_strand_;
    ApiHandler api_handler_;
};

}  // namespace http_handler

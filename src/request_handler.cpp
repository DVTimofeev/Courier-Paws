#include "request_handler.h"

namespace http_handler {

RequestHandler::RequestHandler(Application& app, const fs::path& root_path) 
        : root_path_(root_path)
        , api_strand_(app.GetApiStrand())
        , api_handler_(app)
{}

std::vector<std::string> RequestHandler::ParseTarget(std::string_view target) const{
    std::vector<std::string> parsed_target;
    for (auto&& ch : target)
    {
        if (ch == '/'){
            parsed_target.emplace_back();
        } else {
            parsed_target.back().push_back(ch);
        } 
    }
    return parsed_target;
    
}

StringResponse RequestHandler::ReportServerError(unsigned version, bool keep_alive) const {
    StringResponse response(http::status::bad_request, version);
    response.keep_alive(keep_alive);
    response.set(http::field::cache_control, "no-cache"); 
    response.set(http::field::content_type, ContentType::APP_JSON);
    std::string string_body = R"({"code":"badRequest", "message":"Wrong version of api"})";
    response.body() = string_body;
    response.content_length(string_body.size());
    return response;
}

FileResponse RequestHandler::MakeFileResponse(const StringRequest& req) const {
    FileResponse response(http::status::ok, req.version());
    response.keep_alive(req.keep_alive());
    fs::path file_path = root_path_;
    auto target = req.target();
    if (target == "/") target = "/index.html";
    file_path += target;
    std::string error_path = root_path_.string() + "/error.txt";
    
    http::file_body::value_type file;
    if (file_path > root_path_){
        response.set(http::field::content_type, GetContentType(file_path.extension().string()));
        if (boost::system::error_code ec; file.open(file_path.c_str(), beast::file_mode::read, ec), ec) {
            response.set(http::field::content_type, ContentType::TEXT_PLAIN);
            response.result(http::status::not_found);
            file.open(error_path.c_str(), beast::file_mode::read, ec);
            // std::cout << file.is_open() << root_path_.string() << std::endl; // TODO файлы с ключем и кошельком не открываются (их нет)

        } 
    } else {
        boost::system::error_code ec;
        response.set(http::field::content_type, ContentType::TEXT_PLAIN);
        response.result(http::status::bad_request);
        file.open(error_path.c_str(), beast::file_mode::read, ec);
    }
    response.body() = std::move(file);
    file.close();
    response.prepare_payload();
    return response;
}

std::string_view RequestHandler::GetContentType(std::string_view file_extention) const {
    if (file_extention == ".htm" || file_extention == ".html") return ContentType::TEXT_HTML;
    if (file_extention == ".css") return ContentType::TEXT_CSS;
    if (file_extention == ".txt") return ContentType::TEXT_PLAIN;
    if (file_extention == ".js") return ContentType::TEXT_JS;
    if (file_extention == ".json") return ContentType::APP_JSON;
    if (file_extention == ".xml") return ContentType::APP_XML;
    if (file_extention == ".png") return ContentType::IMAGE_PNG;
    if (file_extention == ".jpg" || file_extention == ".jpe" || file_extention == ".jpeg") return ContentType::IMAGE_JPEG;
    if (file_extention == ".gif") return ContentType::IMAGE_GIF;
    if (file_extention == ".bmp") return ContentType::IMAGE_BMP;
    if (file_extention == ".ico") return ContentType::IMAGE_ICO;
    if (file_extention == ".tiff" || file_extention == ".tif") return ContentType::IMAGE_TIFF;
    if (file_extention == ".svg" || file_extention == ".svgz") return ContentType::IMAGE_SVG_XML;
    if (file_extention == ".mp3") return ContentType::AUDIO_MPEG;
    return ContentType::APP_OCTET_STREAM;
}
}  // namespace http_handler

#include <response.h>

namespace http_handler{

StringResponse Response::MakeBadRequestInvalidArgument(std::string_view err_msg) {
    StringResponse response;
    response.result(http::status::bad_request);
    auto body = (boost::format(R"({"code":"badRequest", "message":"%1%"})") % err_msg).str();
    response.body() = body;
    response.content_length(body.size());
    response.set(http::field::cache_control, "no-cache"); 
    response.set(http::field::content_type, ContentType::APP_JSON);

    return response;
}
StringResponse Response::MakeMethodNotAllowed(std::string_view err_msg, std::string_view allowed_methods) {
    StringResponse response;
    response.result(http::status::method_not_allowed);
    response.set(http::field::allow, allowed_methods);
    auto body = (boost::format(R"({"code":"invalidMethod", "message":"%1%"})") % err_msg).str();
    response.body() = body;
    response.content_length(body.size());
    response.set(http::field::cache_control, "no-cache"); 
    response.set(http::field::content_type, ContentType::APP_JSON);

    return response;
}

StringResponse Response::MakeJSON(http::status status, std::string_view err_code, std::string_view err_msg) {
    StringResponse response;
    response.result(status);
    auto body = (boost::format(R"({"code":"%1%", "message":"%2%"})") % err_code % err_msg).str();
    response.body() = body;
    response.content_length(body.size());
    response.set(http::field::cache_control, "no-cache"); 
    response.set(http::field::content_type, ContentType::APP_JSON);

    return response;
}

} // namespace http_handler
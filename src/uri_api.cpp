#include "uri_api.h"

namespace uri_api {

UriElement& UriElement::SetAllowedMethods(std::vector<http::verb> methods, std::string_view method_error_message, std::string_view allowed_methods) {
    methods_.data_ = std::move(methods);
    methods_.error_ = method_error_message;
    methods_.allowed_ = allowed_methods;

    return *this;
}

UriElement& UriElement::SetNeedAuthorisation(bool need_authorize) {
    authorize_.need_ = need_authorize;

    return *this;
}

UriElement& UriElement::SetProcessFunction(FunctionWithAuthorize func) {
    process_function_ = std::move(func);

    return *this;
}

UriElement& UriElement::SetProcessFunction(FunctionWithoutAuthorize func) {
    process_function_without_authorize_ = std::move(func);

    return *this;
}

UriElement& UriElement::SetContentType(std::string_view type, std::string_view error_message) {
    content_type_.need_to_check_ = true;
    content_type_.value_ = type;
    content_type_.error_ = error_message;

    return *this;
}

http_handler::StringResponse UriElement::ProcessRequest(StringRequest req) 
{
    if (methods_.data_.empty() || std::find(methods_.data_.begin(), methods_.data_.end(), req.method()) != methods_.data_.end()) 
    {
        if (content_type_.need_to_check_ && content_type_.value_ != req.base()[http::field::content_type]) 
        {
            return http_handler::Response::MakeBadRequestInvalidArgument(content_type_.error_);
        }

        if (authorize_.need_) {
            return security::ExecuteAuthorized(req, [&](const Token& token, std::string_view body) {
                return process_function_(token, body);
            });
        }

        auto stop = req.target().find('?');
        if (stop != std::string::npos) {
            return process_function_without_authorize_(req.target().substr(stop + 1));
        }

        return process_function_without_authorize_(req.body());
    }

    return http_handler::Response::MakeMethodNotAllowed(methods_.error_, methods_.allowed_);
}

UriElement* UriData::AddEndpoint(std::string_view uri) {
        auto [it, ret] = data_.try_emplace(std::string(uri));

        return &it->second;
    }

http_handler::StringResponse UriData::Process(StringRequest req) {
    auto stop = req.target().find('?');
    std::string target = std::string(req.target().substr(0, stop));
    auto it = data_.find(target);
    if (it != data_.end()) {
        return it->second.ProcessRequest(req);
    }

    return http_handler::Response::MakeJSON(http::status::bad_request, ErrorCode::BAD_REQUEST, ErrorMessage::INVALID_ENDPOINT);
}

} // namespace uri_api
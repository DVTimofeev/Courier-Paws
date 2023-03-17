#pragma once

#define BOOST_BEAST_USE_STD_STRING_VIEW

#include <string_view>
#include <string>
#include <functional>
#include <unordered_map>
#include <vector>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

#include "response.h"
#include "token.h"

namespace uri_api {

namespace beast = boost::beast;
namespace http = beast::http;
using Token = security::PlayerTokens::Token;
using FunctionWithAuthorize = std::function<http_handler::StringResponse(const Token&, std::string_view body)>;
using FunctionWithoutAuthorize = std::function<http_handler::StringResponse(std::string_view body)>;
using StringRequest = http::request<http::string_body>;


class UriElement {
    struct AllowedMethods {
        std::vector<http::verb> data_;
        std::string_view error_;
        std::string_view allowed_;

        AllowedMethods()
        : data_()
        , error_()
        , allowed_()
        {};
    };

    struct AuthorizeData {
        bool need_;

        AuthorizeData()
        : need_()
        {};
    };

    struct ContentType {
        bool need_to_check_;
        std::string_view error_;
        std::string_view value_;

        ContentType()
        : need_to_check_()
        , error_()
        , value_()
        {};
    };

public:
    UriElement()
    : methods_()
    , authorize_()
    , content_type_()
    {};

    UriElement& SetAllowedMethods(std::vector<http::verb> methods, std::string_view method_error_message, std::string_view allowed_methods);
    UriElement& SetNeedAuthorisation(bool need_authorize);
    UriElement& SetProcessFunction(FunctionWithAuthorize func);
    UriElement& SetProcessFunction(FunctionWithoutAuthorize func);
    UriElement& SetContentType(std::string_view type, std::string_view error_message);
    http_handler::StringResponse ProcessRequest(StringRequest req);

private:
    AllowedMethods methods_;
    AuthorizeData authorize_;
    ContentType content_type_;
    FunctionWithAuthorize process_function_;
    FunctionWithoutAuthorize process_function_without_authorize_;
};

class UriData {
public:
    UriData() = default;
    UriElement* AddEndpoint(std::string_view uri);
    http_handler::StringResponse Process(StringRequest req);

private:
    std::unordered_map<std::string, UriElement> data_;
};

} // namespace uri_api
#pragma once

#define BOOST_BEAST_USE_STD_STRING_VIEW

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/format.hpp>

#include "constants.h"

namespace http_handler {

namespace beast = boost::beast;
namespace http = beast::http;
using StringResponse = http::response<http::string_body>;
using namespace std::literals;

class Response {
public:
    static StringResponse MakeBadRequestInvalidArgument(std::string_view err_msg);
    static StringResponse MakeMethodNotAllowed(std::string_view err_msg, std::string_view allowed_methods);
    static StringResponse MakeJSON(http::status status, std::string_view err_code, std::string_view err_msg);
};

} // namespace http_handler
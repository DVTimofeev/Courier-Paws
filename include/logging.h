#pragma once
#include <chrono>

#include <boost/log/trivial.hpp>     // для BOOST_LOG_TRIVIAL
#include <boost/log/core.hpp>        // для logging::core
#include <boost/log/expressions.hpp> // для выражения, задающего фильтр 
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/date_time.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>

#include <boost/json.hpp>


// BOOST_LOG_ATTRIBUTE_KEYWORD(line_id, "LineID", unsigned int)
// BOOST_LOG_ATTRIBUTE_KEYWORD(timestamp, "TimeStamp", boost::posix_time::ptime)
// BOOST_LOG_ATTRIBUTE_KEYWORD(file, "File", std::string)
// BOOST_LOG_ATTRIBUTE_KEYWORD(line, "Line", int) 
// namespace logging{
BOOST_LOG_ATTRIBUTE_KEYWORD(additional_data, "AdditionalData", boost::json::value)
using namespace std::literals;

namespace logging = boost::log;
namespace keywords = boost::log::keywords;
namespace sinks = boost::log::sinks;
namespace expr = boost::log::expressions;
namespace json = boost::json;


void MyFormatter(logging::record_view const& rec, logging::formatting_ostream& strm);

static std::string GetTimestamp();
// } // namespace logging
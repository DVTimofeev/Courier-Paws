#include "logging.h"

// namespace logging{
// void MyFormatter(logging::record_view const& rec, logging::formatting_ostream& strm) {
//     strm << rec[additional_data] << rec[expr::smessage] << std::endl;
// }

void MyFormatter(logging::record_view const& rec, logging::formatting_ostream& strm) {
    json::value log{
        {"timestamp", GetTimestamp()},
        {"data", *rec[additional_data]},
        {"message", *rec[expr::smessage]}
    };

    strm << log << std::endl;
}


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
// } // namespace logging
#include "url_parser.h"

UrlParser::UrlParser(std::string url){
    boost::regex ex("(http|https)://([^/ :]+):?([^/ ]*)(/?[^ #?]*)\\x3f?([^ #]*)#?([^ ]*)");
    boost::cmatch what;
    if(regex_match(url.c_str(), what, ex)) {
        std::string protocol_ = std::string(what[1].first, what[1].second);
        std::string domain_   = std::string(what[2].first, what[2].second);
        std::string port_     = std::string(what[3].first, what[3].second);
        std::string path_     = std::string(what[4].first, what[4].second);
        std::string query_    = std::string(what[5].first, what[5].second);
        std::string fragment_ = std::string(what[6].first, what[6].second);
        
        for (auto&& ch : path_) {
            if (ch == '/'){
                parsedPath_.emplace_back();
            } else {
                parsedPath_.back().push_back(ch);
            } 
        }
    }
}

std::string UrlParser::GetProtocol(){return protocol_;}
std::string UrlParser::GetDomain(){return domain_;}
std::string UrlParser::GetPort(){return port_;}   
std::string UrlParser::GetPath(){return path_;}   
std::string UrlParser::GetQuery(){return query_;}
std::string UrlParser::GetFragment(){return fragment_;}
std::vector<std::string> UrlParser::GetParsedPath(){return parsedPath_;}



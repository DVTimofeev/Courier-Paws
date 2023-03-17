#include <string>
#include <vector>
#include <iostream>

#include <boost/regex.hpp>

class UrlParser{
public:
    explicit UrlParser(std::string url);
    std::string GetProtocol();
    std::string GetDomain(); 
    std::string GetPort();   
    std::string GetPath();   
    std::string GetQuery();  
    std::string GetFragment();
    std::vector<std::string> GetParsedPath();
private: 
    std::string protocol_;
    std::string domain_; 
    std::string port_;   
    std::string path_;   
    std::string query_;  
    std::string fragment_;
    std::vector<std::string> parsedPath_;
};
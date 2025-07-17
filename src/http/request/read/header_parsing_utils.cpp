#include <sstream>
#include "header_parseing_utils.hpp"
namespace http {
namespace parser {
    
std::string extractHost(const RawHeaders& headers) {
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
        if (it->first == "Host") {
            return it->second;
        }
    }
    return "";
}

std::string extractUri(const std::string& requestLine) {
    std::istringstream iss(requestLine);
    std::string method;
    std::string uri;
    std::string version;
    iss >> method >> uri >> version;
    return uri;
}

} // namespace parser
} // namespace http

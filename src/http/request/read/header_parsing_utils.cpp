#include <sstream>
#include "header_parsing_utils.hpp"

namespace http {
namespace parser {

std::string extractHeader(const RawHeaders& headers, const std::string& key) {
    RawHeaders::const_iterator it = headers.find(key);
    if (it != headers.end()) {
        return it->second;
    }
    return "";
}
    
std::string extractHost(const RawHeaders& headers) {
    return extractHeader(headers, "Hosst");
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

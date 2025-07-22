#include "header_parsing_utils.hpp"

#include <algorithm>
#include <sstream>

#include "http/request/read/body.hpp"
#include "utils/string.hpp"

namespace http {
namespace parser {

std::string extractHeader(const RawHeaders& headers, const std::string& key) {
    const std::string lowercaseKey = utils::toLower(key);

    for (RawHeaders::const_iterator it = headers.begin(); it != headers.end();
         ++it) {
        if (utils::toLower(it->first) == lowercaseKey) {
            return it->second;
        }
    }
    return std::string();
}

std::string extractHost(const RawHeaders& headers) {
    const std::string value = extractHeader(headers, "Host");
    return utils::trim(value);
}

std::string extractUri(const std::string& requestLine) {
    std::istringstream iss(requestLine);
    std::string method;
    std::string uri;
    std::string version;
    iss >> method >> uri >> version;
    return uri;
}

bool hasBody(const RawHeaders& headers) {
    // Content-LengthまたはTransfer-Encoding*
    // chungedのいずれががあればボディあり

    RawHeaders::const_iterator it;

    it = headers.find("Content-Length");
    if (it != headers.end()) {
        return true;
    }
    it = headers.find("Transfer-Encoding");
    if (it != headers.end()) {
        std::string value = it->second;
        std::transform(value.begin(), value.end(), value.begin(), ::tolower);
        if (value.find("chunked") != std::string::npos) {
            return true;
        }
    }
    return false;
}

http::BodyEncodingType detectEncoding(const RawHeaders& headers) {
    for (RawHeaders::const_iterator it = headers.begin(); it != headers.end();
         ++it) {
        const std::string key = utils::toLower(it->first);
        const std::string val = utils::toLower(it->second);

        if (key == "content-length") {
            return http::kContentLength;
        }
        if (key == "transfer-encoding" &&
            val.find("chunked") != std::string::npos) {
            return http::kChunked;
        }
    }
    return http::kNone;
}

std::size_t extractContentLength(const RawHeaders& headers) {
    const RawHeaders::const_iterator it = headers.find("Content-Length");
    if (it == headers.end()) {
        return 0;
    }

    const std::string& value = it->second;
    char* end = NULL;
    const unsigned long result = std::strtoul(value.c_str(), &end, NUMBER);

    if (*end != '\0') {
        return 0;
    }
    return static_cast<std::size_t>(result);
}

}  // namespace parser
}  // namespace http

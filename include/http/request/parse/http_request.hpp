#pragma once
#include <string>
#include <vector>
#include "header.hpp"        // RawHeaders を含む
#include "context/locationContext.hpp"
#include "raw_headers.hpp"

namespace http {
namespace parse {

class HttpRequest {
   public:
    // void setMethod(const std::string&) {}
    // void setUri(const std::string&) {}
    // void setVersion(const std::string&) {}
    // void setHeaders(const RawHeaders&) {}
    // void setBody(const std::vector<char>&) {}
    // void setLocation(const LocationContext&) {}
    // void setQueryString(const std::string&) {}
};

}  // namespace parse
}  // namespace http
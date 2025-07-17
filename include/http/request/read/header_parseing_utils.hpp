#pragma once

#include <string>
#include "raw_headers.hpp"

namespace http {
namespace parser {

    std::string extractHost(const RawHeaders& headers);
    std::string extractUri(const std::string& requestLine);

} // namespase parser
} // namespase http
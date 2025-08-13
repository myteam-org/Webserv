#pragma once

#include <string>

namespace http {
    enum HttpMethod {
        kMethodGet,
        kMethodPost,
        kMethodDelete,
        kMethodPut,
        kMethodHead,
        kMethodOptions,
        kMethodTrace,
        kMethodConnect,
        kMethodPatch,
        kMethodUnknown,
    };

    HttpMethod httpMethodFromString(const std::string &method);
    std::string httpMethodToString(HttpMethod method);
} //namespace http

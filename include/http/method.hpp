#pragma once

#include <string>

namespace http {
    enum HttpMethod {
        kMethodUnknown,
        kMethodGet,
        kMethodPost,
        kMethodPut,
        kMethodDelete,
        kMethodHead,
        kMethodOptions,
        kMethodTrace,
        kMethodConnect,
        kMethodPatch,
    };

    HttpMethod httpMethodFromString(const std::string &method);
    std::string httpMethodToString(HttpMethod method);
} //namespace http

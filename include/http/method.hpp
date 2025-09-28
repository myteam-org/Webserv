#pragma once
#include <string>

namespace http {
    // HTTPメソッド列挙（未対応メソッドも含む）
    enum HttpMethod {
        kMethodUnknown,
        kMethodGet,
        kMethodPost,
        kMethodDelete,
        kMethodPut,
        kMethodPatch,
        kMethodHead,
        kMethodOptions,
        kMethodTrace,
        kMethodConnect
    };

    HttpMethod httpMethodFromString(const std::string &method);
    std::string httpMethodToString(const HttpMethod method);
    bool isMethodImplemented(const HttpMethod method); // サーバ対応判定用
}

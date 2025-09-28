#include "http/method.hpp"

namespace http {
    HttpMethod httpMethodFromString(const std::string &method) {
        if (method == "GET")     return kMethodGet;
        if (method == "POST")    return kMethodPost;
        if (method == "DELETE")  return kMethodDelete;
        if (method == "PUT")     return kMethodPut;
        if (method == "PATCH")   return kMethodPatch;
        if (method == "HEAD")    return kMethodHead;
        if (method == "OPTIONS") return kMethodOptions;
        if (method == "TRACE")   return kMethodTrace;
        if (method == "CONNECT") return kMethodConnect;
        return kMethodUnknown;
    }

    std::string httpMethodToString(const HttpMethod method) {
        switch (method) {
            case kMethodGet:     return "GET";
            case kMethodPost:    return "POST";
            case kMethodDelete:  return "DELETE";
            case kMethodPut:     return "PUT";
            case kMethodPatch:   return "PATCH";
            case kMethodHead:    return "HEAD";
            case kMethodOptions: return "OPTIONS";
            case kMethodTrace:   return "TRACE";
            case kMethodConnect: return "CONNECT";
            default:             return "UNKNOWN";
        }
    }

    // GET, POST, DELETE だけtrue。他は未実装（501返却対象）
    bool isMethodImplemented(const HttpMethod method) {
        switch (method) {
            case kMethodGet:
            case kMethodPost:
            case kMethodDelete:
                return true;
            default:
                return false;
        }
    }
} // namespace http

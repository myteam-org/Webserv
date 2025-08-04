#include "http/response/builder.hpp"

#include "http/mime.hpp"
#include "utils/string.hpp"
#include <fstream>
#include <sstream>

namespace http {
    ResponseBuilder::ResponseBuilder() : status_(kStatusOk), httpVersion_("HTTP/1.1"), body_(types::none<std::string>()) {}

    ResponseBuilder::~ResponseBuilder() {}

    Response ResponseBuilder::build() {
        if (body_.isNone()) {
            // 念の為 Content-Length を付ける
            this->header("Content-Length", "0");
        }
        return Response(status_, 
                        // headers_, 
                        body_, httpVersion_);
    }

    ResponseBuilder &ResponseBuilder::status(const HttpStatusCode status) {
        status_ = status;
        return *this;
    }

    ResponseBuilder &ResponseBuilder::header(const std::string &name, const std::string &value) { // NOLINT
        (void)name;
        (void)value;
        // headers_[name] = value;
        return *this;
    }

    ResponseBuilder &ResponseBuilder::text(const std::string &body, const HttpStatusCode status) {
        body_ = types::some(body);
        return this->status(status)
            .header("Content-Type", "text/plain; charset=UTF-8")
            .header("Content-Length", utils::toString(body.size()));
    }

    ResponseBuilder &ResponseBuilder::html(const std::string &body, const HttpStatusCode status) {
        body_ = types::some(body);
        return this->status(status)
            .header("Content-Type", "text/html; charset=UTF-8")
            .header("Content-Length", utils::toString(body.size()));
    }

    ResponseBuilder &ResponseBuilder::redirect(const std::string &location, const HttpStatusCode status) {
        return this->status(status).header("Location", location);
    }

    ResponseBuilder &ResponseBuilder::file(const std::string &path, HttpStatusCode status) {
        std::ifstream ifs(path.c_str());
        if (!ifs.is_open()) {
            return this->status(kStatusNotFound);
        }

        std::stringstream ss;
        ss << ifs.rdbuf();
        const std::string body = ss.str();

        const std::string mime = getMimeType(path);
        std::ostringstream stream;
        stream << mime << "; charset=UTF-8";
        return this->body(body, status)
            .header("Content-Type", stream.str())
            .header("Content-Length", utils::toString(body.size()));
    }

    ResponseBuilder &ResponseBuilder::body(const std::string &body, const HttpStatusCode status) {
        body_ = types::some(body);
        return this->status(status);
    }
} //namespace http
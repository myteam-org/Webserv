#include "http/response/builder.hpp"

#include <fstream>
#include <sstream>

#include "http/mime.hpp"
#include "utils/string.hpp"

namespace {

// 値の最低限バリデーション（CR/LF禁止）
bool isSafeHeaderValue(const std::string &str) {
    for (std::size_t i = 0; i < str.size(); ++i) {
        const char chr = str[i];
        if (chr == '\r' || chr == '\n') {
            return false;
        }
    }
    return true;
}
}  // anonymous namespace

namespace http {
ResponseBuilder::ResponseBuilder()
    : status_(kStatusOk),
      httpVersion_("HTTP/1.1"),
      body_(types::none<std::string>()) {}

ResponseBuilder::~ResponseBuilder() {}

Response ResponseBuilder::build() {
    const int intStatus = static_cast<int>(status_);
    const bool noBody = (intStatus >= 100 && intStatus < 200) ||
                         intStatus == 204 || intStatus == 205 ||
                         intStatus == 304;
    const bool hasTE = headers_.hasField("Transfer-Encoding");

    if (noBody) {
        body_ = types::none<std::string>();
        if (hasTE) {
            headers_.removeField("Transfer-Encoding");
        }
        headers_.setField("Content-Length", "0");
    } else {
        // TE があるなら CL は削除（併用禁止）
        if (hasTE) {
            if (headers_.hasField("Content-Length")) {
                headers_.removeField("Content-Length");
            }
        } else if (!headers_.hasField("Content-Length")) {
            headers_.setField(
                "Content-Length",
                body_.isNone() ? "0" : utils::toString(body_.unwrap().size()));
        }
    }
    if (!headers_.hasField("Connection")) {
        headers_.setField("Connection", "keep-alive");
    }
    return Response(status_, headers_.getMembers(), body_, httpVersion_);
}

ResponseBuilder &ResponseBuilder::status(const HttpStatusCode status) {
    status_ = status;
    return *this;
}

ResponseBuilder &ResponseBuilder::header(const std::string &name,
                                         const std::string &value) {  // NOLINT
    if (name.empty()) {
        return *this;
    }
    if (!isSafeHeaderValue(value)) {
        return *this;
    }
    headers_.setField(name, value);
    return *this;
}

ResponseBuilder &ResponseBuilder::text(const std::string &body,
                                       const HttpStatusCode status) {
    body_ = types::some(body);
    return this->status(status)
        .header("Content-Type", "text/plain; charset=UTF-8")
        .header("Content-Length", utils::toString(body.size()));
}

ResponseBuilder &ResponseBuilder::html(const std::string &body,
                                       const HttpStatusCode status) {
    body_ = types::some(body);
    return this->status(status)
        .header("Content-Type", "text/html; charset=UTF-8")
        .header("Content-Length", utils::toString(body.size()));
}

// 明示的に「本文なし」にしておくと挙動が安定
ResponseBuilder &ResponseBuilder::redirect(const std::string &location,
                                           const HttpStatusCode status) {
    body_ = types::none<std::string>();
    headers_.removeField("Transfer-Encoding");
    return this->status(status)
        .header("Location", location)
        .header("Content-Length", "0");
}

// バイナリで開く（テキスト変換を防止）
ResponseBuilder &ResponseBuilder::file(const std::string &path,
                                       HttpStatusCode status) {
    std::ifstream ifs(path.c_str(), std::ios::in | std::ios::binary);
    if (!ifs.is_open()) {
        body_ = types::none<std::string>();
        return this->status(kStatusNotFound).header("Content-Length", "0");
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

ResponseBuilder &ResponseBuilder::body(const std::string &body,
                                       const HttpStatusCode status) {
    body_ = types::some(body);
    this->status(status);
    headers_.setField("Content-Length", utils::toString(body.size()));
    return *this;
}
}  // namespace http

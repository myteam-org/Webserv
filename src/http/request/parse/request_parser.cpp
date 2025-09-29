#include "http/request/parse/request_parser.hpp"

// c++98
#include <sstream>

#include "http/request/read/context.hpp"
#include "http/request/request.hpp"
#include "http/method.hpp"
#include "utils/string.hpp"
#include "utils/types/try.hpp"
#include "utils/types/option.hpp"
#include "utils/logger.hpp"

namespace http {
namespace parse {

RequestParser::RequestParser(http::ReadContext& ctx)
    : ctx_(&ctx),
      requestLineParsed_(false),
      headersParsed_(false),
      bodyParsed_(false),
      method_(kMethodUnknown),
      requestTarget_(),
      pathOnly_(),
      queryString_(),
      version_("HTTP/1.1"),
      headers_(),
      body_() {
}

RequestParser::~RequestParser() {}

types::Result<types::Unit, error::AppError>
RequestParser::parseRequestLine() {
    if (requestLineParsed_) {
        return types::ok(types::Unit());
    }
    const std::string& line = ctx_->getRequestLine();
    std::istringstream iss(line);

    std::string methodStr;
    std::string target;
    std::string ver;
    if (!(iss >> methodStr >> target >> ver)) {
        return types::err(error::kBadRequest);
    }

    method_ = httpMethodFromString(methodStr);
    if (method_ == kMethodUnknown) {
        return types::err(error::kBadRequest);
    }
    if (ver.compare(0, 5, "HTTP/") != 0) {
        return types::err(error::kBadRequest);
    }
    version_ = ver;

    types::Result<types::Unit, error::AppError> dec =
        decodeAndNormalizeTarget(target);
    if (dec.isErr()) {
        return dec;
    }

    requestLineParsed_ = true;
    return types::ok(types::Unit());
}

types::Result<types::Unit, error::AppError>
RequestParser::parseHeaders() {
    if (headersParsed_) {
        return types::ok(types::Unit());
    }
    headers_ = ctx_->getHeaders(); // 既にキーは lower-case で格納されている想定

    for (RawHeaders::const_iterator it = headers_.begin(); it != headers_.end(); ++it) {
    }

    if (!checkMissingHost()) {
        return types::err(error::kBadRequest);
    }
    if (!validateContentLength()) {
        return types::err(error::kBadRequest);
    }
    if (!validateTransferEncoding()) {
        return types::err(error::kBadRequest);
    }
    headersParsed_ = true;
    return types::ok(types::Unit());
}

types::Result<types::Unit, error::AppError>
RequestParser::parseBody() {
    if (bodyParsed_) {
        return types::ok(types::Unit());
    }
    const std::string& b = ctx_->getBody();
    body_.assign(b.begin(), b.end());
    bodyParsed_ = true;
    return types::ok(types::Unit());
}

types::Result<const LocationContext*, error::AppError>
RequestParser::chooseLocation(const std::string& pathOnly) const {
    const ServerContext& srv = ctx_->getServer();
    const LocationContextList locations = srv.getLocation();
    LocationContextList::const_iterator it = locations.begin();
    const LocationContext* chosen = NULL;
    for (; it != locations.end(); ++it) {
        const std::string& locPath = it->getPath();
        if (pathOnly.find(locPath) == 0) {
            if (chosen == NULL || locPath.size() > chosen->getPath().size()) {
                chosen = &(*it);
            }
        }
    }
    if (chosen == NULL) {
        if (!locations.empty()) {
            chosen = &(*locations.begin());
        }
    }
    if (chosen == NULL) {
        return types::err(error::kBadRequest);
    }
    return types::ok(chosen);
}

const std::string& RequestParser::getPathOnly() const { return pathOnly_; }
const std::string& RequestParser::getQueryString() const { return queryString_; }
const std::string& RequestParser::getRequestTarget() const { return requestTarget_; }
http::HttpMethod RequestParser::getMethod() const { return method_; }

types::Result<http::Request, error::AppError>
RequestParser::buildRequest() const {
    // request line / headers は最低限必要
    if (!requestLineParsed_ || !headersParsed_) {
        return types::err(error::kBadRequest);
    }
    // body はメソッドや Transfer-Encoding により後で読むケースもあるので必須とはしない場合もある
    // 必須にしたい場合は !bodyParsed_ チェックを追加

    types::Result<const LocationContext*, error::AppError> locResult =
        chooseLocation(pathOnly_);
    if (locResult.isErr()) {
        return types::err(locResult.unwrapErr());
    }
    const LocationContext* loc = locResult.unwrap();

    // Request は 8 引数 (pathOnly, queryString を明示的に渡す)
    http::Request req(
        method_,
        requestTarget_,
        pathOnly_,
        queryString_,
        headers_,
        body_,
        &ctx_->getServer(),
        loc
    );
    return types::ok(req);
}

// ---- 内部検証ヘルパ ----
bool RequestParser::checkMissingHost() const {
    RawHeaders::const_iterator it = headers_.find("host");
    return it != headers_.end();
}

bool RequestParser::validateContentLength() const {
    RawHeaders::const_iterator it = headers_.find("content-length");
    if (it == headers_.end()) {
        return true;
    }
    const std::string& v = it->second;
    if (v.empty()) {
        return false;
    }
    for (std::string::size_type i = 0; i < v.size(); ++i) {
        if (v[i] < '0' || v[i] > '9') {
            return false;
        }
    }
    return true;
}

bool RequestParser::validateTransferEncoding() const {
    return true; // 必要なら将来 chunked 等の厳密判定を実装
}

types::Result<types::Unit, error::AppError>
RequestParser::decodeAndNormalizeTarget(const std::string& target) {
    requestTarget_ = target;
    std::string::size_type q = target.find('?');
    if (q == std::string::npos) {
        pathOnly_ = target;
        queryString_ = "";
    } else {
        pathOnly_ = target.substr(0, q);
        queryString_ = target.substr(q + 1);
    }
    if (pathOnly_.empty()) {
        pathOnly_ = "/";
    }
    return types::ok(types::Unit());
}

} // namespace parse
} // namespace http

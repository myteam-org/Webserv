#include "http/request/parse/request_parser.hpp"

#include <sstream>
#include <string>

#include "context.hpp"
#include "locationContext.hpp"
#include "request.hpp"
#include "utils/path.hpp"
#include "utils/string.hpp"
#include "utils/types/either.hpp"
#include "utils/types/result.hpp"
#include "utils/url.hpp"

namespace http {
namespace parse {

static const std::size_t kMaxRecommendedRequestLineLength = 8000;

RequestParser::RequestParser(http::ReadContext& ctx) : ctx_(&ctx) {}

RequestParser::~RequestParser() {}

types::Result<types::Unit, error::AppError> RequestParser::parseRequestLine() {
    const std::string& line = ctx_->getRequestLine();
    if (!utils::endsWith(line, "\r\n")) {
        return ERR(error::kBadRequest);
    }
    std::istringstream iss(line);
    std::string method;
    std::string requestTarget;
    std::string version;
    if (!(iss >> method >> requestTarget >> version)) {
        return ERR(error::kBadRequest);
    }
    if (method == "GET") {
        method_ = kMethodGet;
    } else if (method == "POST") {
        method_ = kMethodPost;
    } else if (method == "DELETE") {
        method_ = kMethodDelete;
    } else {
        return ERR(error::kBadMethod);
    }
    if (requestTarget.length() > kMaxRecommendedRequestLineLength) {
        return ERR(error::kUriTooLong);
    }
    const std::size_t kHttpVersionPreFixLen = 8;
    if (version.substr(0, kHttpVersionPreFixLen) != "HTTP/1.1") {
        return ERR(error::kBadHttpVersion);
    }
    const types::Result<types::Unit, error::AppError> norm = 
        decodeAndNormalizeTarget(requestTarget);
    if (norm.isErr()) {
        return norm;
    }
    version_ = version;
    return OK(types::Unit());
}

types::Result<types::Unit, error::AppError>
RequestParser::decodeAndNormalizeTarget(const std::string& requestTarget) {
    pathOnly_.clear();
    queryString_.clear();
    std::string rawPath = requestTarget;
    const std::size_t qMark = requestTarget.find('?');
    if (qMark != std::string::npos) {
        rawPath = requestTarget.substr(0, qMark);
        queryString_ = requestTarget.substr(qMark + 1);
    }
    std::string decodePath;
    if (!utils::url::decodeStrict(pathOnly_, decodePath)) {
        return ERR(error::kBadRequest);
    }
    pathOnly_ = utils::path::removeDotSegments(decodePath);
    if (pathOnly_.empty()) {
        pathOnly_ = "/";
    }
    requestTarget_ = requestTarget;
    return OK(types::Unit());
}

types::Result<types::Unit, error::AppError> RequestParser::parseHeaders() {
    headers_ = ctx_->getHeaders();

    if (checkMissingHost()) {
        return ERR(error::kMissingHost);
    }
    const bool hasCL = headers_.find("Content-Length") != headers_.end();
    if (hasCL && !validateContentLength()) {
        return ERR(error::kInvalidContentLength);
    }
    const bool hasTE = headers_.find("Transfer-Encoding") != headers_.end();
    if (hasTE && !validateTransferEncoding()) {
        return ERR(error::kInvalidTransferEncoding);
    }
    if (hasCL && hasTE) {
        return ERR(error::kHasContentLengthAndTransferEncoding);
    }
    return OK(types::Unit());
}

bool RequestParser::checkMissingHost() const {
    return headers_.find("Host") == headers_.end();
}

bool RequestParser::validateContentLength() const {
    const RawHeaders::const_iterator it = headers_.find("Content-Length");
    if (it == headers_.end()) {
        return false;
    }
    const std::string& val = it->second;
    return !val.empty() && !utils::containsNonDigit(val);
}

bool RequestParser::validateTransferEncoding() const {
    const RawHeaders::const_iterator it = headers_.find("Transfer-Encoding");
    if (it == headers_.end()) {
        return false;
    }
    const std::string& val = it->second;
    return !val.empty() && utils::toLower(val) == "chunked";
}

types::Result<types::Unit, error::AppError> RequestParser::parseBody() {
    const std::string& raw = ctx_->getBody();
    // バイナリ対応のため vector<char> に詰め直す    body_.assign(raw.begin(),
    // raw.end());
    return OK(types::Unit());
}

types::Result<Request, error::AppError> RequestParser::buildRequest() const {
    const std::string uri = requestTarget_;
    const types::Result<const LocationContext*, error::AppError> result =
        chooseLocation(uri);
    if (result.isErr()) {
        return ERR(error::kBadLocationContext);
    }
    const LocationContext* location = result.unwrap();

    // std::string queryString;
    // std::string pathOnly = uri;
    // const std::size_t queryMarkPos = uri.find('?');
    // if (queryMarkPos != std::string::npos) {
    //     pathOnly = uri.substr(0, queryMarkPos);
    //     queryString = uri.substr(queryMarkPos + 1);
    // }
    const Request req(method_, requestTarget_, pathOnly_, queryString_, headers_, body_, &ctx_->getServer(),
                      result.unwrap());
    return OK(req);
}

types::Result<const LocationContext*, error::AppError>
RequestParser::chooseLocation(const std::string& uri) const {
    if (!ctx_) {
        return types::err(error::kBadRequest);
    }
    const ServerContext& server = ctx_->getServer();
    const std::vector<LocationContext>& locations = server.getLocation();
    const LocationContext* bestMatch = NULL;
    std::size_t longest = 0;

    for (std::size_t i = 0; i < locations.size(); ++i) {
        const std::string& path = locations[i].getPath();
        if (uri.compare(0, path.size(), path) == 0) {
            if (path.size() > longest) {
                longest = path.size();
                bestMatch = &locations[i];
            }
        }
    }
    if (bestMatch) {
        return types::ok(bestMatch);
    }
    return types::err(error::kBadRequest);
}

}  // namespace parse
}  // namespace http

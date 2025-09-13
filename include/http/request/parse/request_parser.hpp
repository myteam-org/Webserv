#pragma once
// c++98

#include <string>
#include <vector>

#include "http/method.hpp"
#include "http/request/read/raw_headers.hpp"
#include "utils/types/error.hpp"
#include "utils/types/result.hpp"
#include "utils/types/option.hpp"

class ServerContext;
class LocationContext;
class DocumentRootConfig;

namespace http {
class ReadContext;
class Request;
}

namespace http {
namespace parse {

/*
 * RequestParser
 *  - ReadContext に蓄積された request line / headers / body を参照し、
 *    HTTP仕様に沿った基礎的な検証と正規化を行い Request を構築する補助クラス。
 *  - 状態遷移は RequestReader が管理し、各段階で必要に応じて
 *    parseRequestLine / parseHeaders / parseBody / buildRequest を呼ぶ。
 *  - 冪等性：同じ parseXXX が複数回呼ばれても 2 回目以降は何もせず成功扱い。
 */
class RequestParser {
public:
    explicit RequestParser(http::ReadContext& ctx);
    ~RequestParser();

    types::Result<types::Unit, error::AppError> parseRequestLine();
    types::Result<types::Unit, error::AppError> parseHeaders();
    types::Result<types::Unit, error::AppError> parseBody();

    types::Result<const LocationContext*, error::AppError>
        chooseLocation(const std::string& pathOnly) const;

    const std::string& getPathOnly() const;
    const std::string& getQueryString() const;
    const std::string& getRequestTarget() const;
    http::HttpMethod    getMethod() const;

    types::Result<http::Request, error::AppError> buildRequest() const;

private:
    // 内部ヘルパ
    bool checkMissingHost() const;
    bool validateContentLength() const;
    bool validateTransferEncoding() const;
    types::Result<types::Unit, error::AppError>
        decodeAndNormalizeTarget(const std::string& target);

    http::ReadContext* ctx_;
    bool requestLineParsed_;
    bool headersParsed_;
    bool bodyParsed_;

    http::HttpMethod method_;
    std::string requestTarget_;
    std::string pathOnly_;
    std::string queryString_;
    std::string version_;

    RawHeaders headers_;
    std::vector<char> body_;
};

} // namespace parse
} // namespace http

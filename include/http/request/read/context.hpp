#pragma once
// c++98

#include <string>

#include "http/request/read/state.hpp"
#include "http/request/read/raw_headers.hpp"
#include "http/config/config_resolver.hpp"
#include "utils/types/option.hpp"
#include "utils/types/result.hpp"
#include "utils/types/error.hpp"

class ServerContext;

namespace http {

class IState;

class ReadContext {
public:
    ReadContext(config::IConfigResolver& resolver, IState* initial);
    ~ReadContext();

    HandleResult handle(ReadBuffer& buf);
    void changeState(IState* next);

    const IState* getState() const;

    // 進捗フラグ
    bool isRequestLineComplete() const;
    bool isHeadersComplete() const;
    bool isBodyComplete() const;

    // データ設定/取得
    void setRequestLine(const std::string& line);
    const std::string& getRequestLine() const;

    void setHeaders(const RawHeaders& headers);
    const RawHeaders& getHeaders() const;

    void setBody(const std::string& body);
    const std::string& getBody() const;

    types::Option<IState*> createReadingBodyState(const RawHeaders& headers) const;

    void setServer(const ServerContext& server);
    const ServerContext& getServer() const;
    bool hasServer() const;

    // ★ 復活: ConfigResolver へのアクセス
    config::IConfigResolver& getConfigResolver() const;

    void resetForNext();

private:
    IState* state_;
    http::config::IConfigResolver& resolver_;

    std::string requestLine_;
    RawHeaders headers_;
    std::string body_;

    bool hasRequestLine_;
    bool hasHeaders_;
    bool hasBody_;

    const ServerContext* server_;

    void markRequestLine();
    void markHeaders();
    void markBody();
};

} // namespace http

#include "http/request/read/context.hpp"

// c++98
#include "http/request/read/body.hpp"
#include "http/request/read/header.hpp"
#include "http/request/read/header_parsing_utils.hpp"
#include "http/request/read/line.hpp"
#include "config/context/serverContext.hpp"
#include "http/config/config_resolver.hpp"

namespace http {

ReadContext::ReadContext(config::IConfigResolver& resolver, IState* initial)
    : state_(initial),
      resolver_(resolver),
      requestLine_(),
      headers_(),
      body_(),
      hasRequestLine_(false),
      hasHeaders_(false),
      hasBody_(false),
      server_(NULL) {}

ReadContext::~ReadContext() {
    delete state_;
}

HandleResult ReadContext::handle(ReadBuffer& buf) {
    if (state_ == NULL) {
        return types::ok(IState::kDone);
    }

    const TransitionResult tr = state_->handle(*this, buf);

    if (tr.getRequestLine().isSome()) {
        requestLine_ = tr.getRequestLine().unwrap();
        markRequestLine();
    }
    if (tr.getHeaders().isSome()) {
        headers_ = tr.getHeaders().unwrap();
        markHeaders();
    }
    if (tr.getBody().isSome()) {
        body_ = tr.getBody().unwrap();
        markBody();
    }

    if (tr.getStatus().isOk() && tr.getStatus().unwrap() == IState::kDone) {
        if (dynamic_cast<ReadingRequestHeadersState*>(state_) != NULL) {
            if (parser::hasBody(headers_)) {
                const types::Option<IState*> opt =
                    createReadingBodyState(headers_);
                if (opt.isSome()) {
                    changeState(opt.unwrap());
                } else {
                    return types::err(error::kBadRequest);
                }
            } else {
                changeState(NULL);
            }
        } else if (dynamic_cast<ReadingRequestBodyState*>(state_) != NULL) {
            changeState(NULL);
        }
    }

    if (tr.getNextState() != NULL) {
        changeState(tr.getNextState());
    }

    return tr.getStatus();
}

void ReadContext::changeState(IState* next) {
    if (state_ != next) {
        delete state_;
        state_ = next;
    }
}

types::Option<IState*> ReadContext::createReadingBodyState(
    const RawHeaders& headers) const {
    const BodyEncodingType type = parser::detectEncoding(headers);
    if (type == kNone) {
        return types::none<IState*>();
    }

    const ServerContext& config = this->getServer();

    BodyLengthConfig bodyConfig;
    bodyConfig.contentLength = parser::extractContentLength(headers);
    bodyConfig.clientMaxBodySize = config.getClientMaxBodySize();

    IState* next = new ReadingRequestBodyState(type, bodyConfig);
    return types::some(next);
}

const IState* ReadContext::getState() const { return state_; }

bool ReadContext::isRequestLineComplete() const { return hasRequestLine_; }
bool ReadContext::isHeadersComplete() const { return hasHeaders_; }
bool ReadContext::isBodyComplete() const {
    if (!hasHeaders_) {
        return false;
    }
    return hasBody_ || !parser::hasBody(headers_);
}

void ReadContext::setRequestLine(const std::string& line) {
    requestLine_ = line;
    markRequestLine();
}

const std::string& ReadContext::getRequestLine() const { return requestLine_; }

void ReadContext::setHeaders(const RawHeaders& headers) {
    headers_ = headers;
    markHeaders();
}

const RawHeaders& ReadContext::getHeaders() const { return headers_; }

void ReadContext::setBody(const std::string& body) {
    body_ = body;
    markBody();
}

const std::string& ReadContext::getBody() const { return body_; }

void ReadContext::setServer(const ServerContext& server) { server_ = &server; }
const ServerContext& ReadContext::getServer() const { return *server_; }
bool ReadContext::hasServer() const { return server_ != NULL; }

config::IConfigResolver& ReadContext::getConfigResolver() const {
    return resolver_;
}

void ReadContext::resetForNext() {
    delete state_;
    state_ = new ReadingRequestLineState();

    requestLine_.clear();
    headers_.clear();
    body_.clear();

    hasRequestLine_ = false;
    hasHeaders_ = false;
    hasBody_ = false;
    server_ = NULL;
}

void ReadContext::markRequestLine() { hasRequestLine_ = true; }
void ReadContext::markHeaders() { hasHeaders_ = true; }
void ReadContext::markBody() { hasBody_ = true; }

} // namespace http

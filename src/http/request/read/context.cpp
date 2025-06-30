#include "context.hpp"
#include "reader.hpp"

namespace http {

ReadContext::ReadContext(IConfigResolver& resolver, IState* initial)
    : state_(initial),
      resolver_(resolver)
      {}

ReadContext::~ReadContext() {
  delete state_;
}

HandleResult ReadContext::handle(ReadBuffer& buf) {
  if (state_ == NULL) {
    return types::ok(IState::kDone);
  }

  TransitionResult tr = state_->handle(buf);

  if (tr.requestLine.isSome()) {
    requestLine_ = tr.requestLine.unwrap();
  }
  if (tr.headers.isSome()) {
    headers_ = tr.headers.unwrap();
  }
  if (tr.body.isSome()) {
    body_ = tr.body.unwrap();
  }

  if (tr.nextState != NULL) {
    delete state_;
    state_ = tr.nextState;
  }

  return tr.status;
}

void ReadContext::changeState(IState* next) {
  delete state_;
  state_ = next;
}

const IState* ReadContext::getState() const { return state_; }

IConfigResolver& ReadContext::getConfigResolver() const {
  return resolver_;
}

const std::string& ReadContext::getRequestLine() const {
  return requestLine_;
}

const RawHeaders& ReadContext::getHeaders() const { return headers_; }

const std::string& ReadContext::getBody() const { return body_; }

}  // namespace http


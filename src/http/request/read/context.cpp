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

  const TransitionResult tr = state_->handle(buf);

  if (tr.getRequestLine().isSome()) {
    requestLine_ = tr.getRequestLine().unwrap();
  }
  if (tr.getHeaders().isSome()) {
    headers_ = tr.getHeaders().unwrap();
  }
  if (tr.getBody().isSome()) {
    body_ = tr.getBody().unwrap();
  }

  if (tr.getNextState() != NULL) {
    delete state_;
    state_ = tr.getNextState();
  }

  return tr.getStatus();
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


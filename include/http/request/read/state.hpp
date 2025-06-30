#pragma once

#include "utils/types/result.hpp"
#include "utils/types/option.hpp"
#include "utils/types/error.hpp"
#include "buffer.hpp"
#include "utils.hpp"

namespace http {

class IState {
 public:
  enum HandleStatus { kSuspend, kDone };
  virtual ~IState() {}
  virtual struct TransitionResult handle(ReadBuffer& buf) = 0;
};

class TransitionResult {
 private:
  IState* nextState_;
  types::Option<std::string> requestLine_;
  types::Option<RawHeaders> headers_;
  types::Option<std::string> body_;
  types::Result<IState::HandleStatus, error::AppError> status_;

 public:
  TransitionResult()
      : nextState_(NULL),
        requestLine_(types::makeNone<std::string>()),
        headers_(types::makeNone<RawHeaders>()),
        body_(types::makeNone<std::string>()),
        status_(types::ok(IState::kSuspend)) {}

  IState* getNextState() const { return nextState_; }
  void setNextState(IState* nextState) { nextState_ = nextState; }

  const types::Option<std::string>& getRequestLine() const { return requestLine_; }
  void setRequestLine(const types::Option<std::string>& requestLine) {
    requestLine_ = requestLine;
  }

  const types::Option<RawHeaders>& getHeaders() const { return headers_; }
  void setHeaders(const types::Option<RawHeaders>& headers) {
    headers_ = headers;
  }

  const types::Option<std::string>& getBody() const { return body_; }
  void setBody(const types::Option<std::string>& body) { body_ = body; }

  const types::Result<IState::HandleStatus, error::AppError>& getStatus() const {
    return status_;
  }
  void setStatus(
      const types::Result<IState::HandleStatus, error::AppError>& status) {
    status_ = status;
  }
};

typedef types::Result<IState::HandleStatus, error::AppError> HandleResult;

}  // namespace http

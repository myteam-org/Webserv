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
  virtual struct TransitionResult handle(const ReadBuffer& buf) const = 0;
};

struct TransitionResult {
  IState* nextState;
  types::Option<std::string> requestLine;
  types::Option<RawHeaders> headers;
  types::Option<std::string> body;
  types::Result<IState::HandleStatus, error::AppError> status;

  TransitionResult()
      : nextState(NULL),
		requestLine(types::makeNone<std::string>()),
		headers(types::makeNone<RawHeaders>()),
		body(types::makeNone<std::string>()),
		status(types::ok(IState::kSuspend)) {}
};

typedef types::Result<IState::HandleStatus, error::AppError> HandleResult;

}  // namespace http

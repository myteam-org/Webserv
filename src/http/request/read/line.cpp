#include "line.hpp"
#include "state.hpp"
#include "header.hpp"
#include "utils/types/try.hpp"
#include "utils/types/result.hpp"
#include "utils/types/option.hpp"
#include "utils/types/error.hpp"

namespace http {

ReadingRequestLineState::ReadingRequestLineState() {}

ReadingRequestLineState::~ReadingRequestLineState() {}

TransitionResult ReadingRequestLineState::handle(ReadBuffer& buf) {
  TransitionResult tr;

  const GetLineResult result = getLine(buf);
  if (!result.canUnwrap()) {
    tr.status = types::Result<IState::HandleStatus, error::AppError>(
        Err<error::AppError>(result.unwrapErr()));
    return tr;
  }

  const types::Option<std::string> lineOpt = result.unwrap();
  if (lineOpt.isNone()) {
    tr.status = types::ok(IState::kSuspend);
    return tr;
  }

  const std::string line = lineOpt.unwrap();

  if (line.empty()) {
    tr.status = types::Result<IState::HandleStatus, error::AppError>(
        Err<error::AppError>(error::kIOUnknown));
    return tr;
  }

  tr.requestLine = types::Option<std::string>(types::some(line));
  // tr.nextState = new ReadingHeadersState();
  tr.status = types::ok(IState::kDone);
  return tr;
}

}  // namespace http

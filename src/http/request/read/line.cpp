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
    tr.setStatus(types::err(result.unwrapErr()));
    return tr;
  }

  const types::Option<std::string> lineOpt = result.unwrap();
  if (lineOpt.isNone()) {
    tr.setStatus(types::ok(IState::kSuspend));
    return tr;
  }

  const std::string line = lineOpt.unwrap();

  if (line.empty()) {
    tr.setStatus(types::err(error::kIOUnknown));
    return tr;
  }

  tr.setRequestLine(types::Option<std::string>(types::some(line)));
  // tr.setNextState(new ReadingHeadersState());
  tr.setStatus(types::ok(IState::kDone));
  return tr;
}

}  // namespace http

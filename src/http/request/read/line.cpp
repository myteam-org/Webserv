#include "buffer.hpp"
#include "request/read/line.hpp"

namespace http {

ReadingRequestLineState::ReadingRequestLineState() {
}

ReadingRequestLineState::~ReadingRequestLineState() {
}

TransitionResult ReadingRequestLineState::handle(
    const ReadBuffer& buf) const {
  TransitionResult tr;
	types::Option<std::string> line = TRY(getLine(buf));

  if (line.isNone()) {
    tr.status = types::ok(
        IState::kSuspend);
    return tr;
  }

  tr.requestLine = line;
  tr.nextState   = new ReadingHeadersState();
  tr.status       = types::ok(IState::kSuspend);
  return tr;
}

}  // namespace http


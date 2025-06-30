#include "buffer.hpp"
#include "try.hpp"
#include "request/read/line.hpp"
#include "request/read/utils.hpp"
#include "request/read/state.hpp"  // [must] ReadingHeadersState のヘッダを追加

namespace http {

ReadingRequestLineState::ReadingRequestLineState() {
}

ReadingRequestLineState::~ReadingRequestLineState() {
}

TransitionResult ReadingRequestLineState::handle(
    ReadBuffer& buf) const { // [must] constを外す（getLineに合わせる）
  TransitionResult tr;
  types::Result<types::Option<std::string>, error::AppError> result = http::getLine(buf);

  if (!result.canUnwrap() || result.unwrap().isNone()) { // [must] getLine失敗または行なし
    tr.status = types::ok(IState::kSuspend);
    return tr;
  }

  tr.requestLine = result.unwrap(); // Option<std::string>型
  tr.nextState   = new ReadingHeadersState();
  tr.status      = types::ok(IState::kSuspend);
  return tr;
}

}  // namespace http

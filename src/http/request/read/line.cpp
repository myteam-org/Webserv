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

// handle関数
// 1. getLine(buf)を使って1行読み取る
//    getLine()はバッファから\r\n区切りで1行読み取る関数
// 2. 読み取りエラーが起きたらTransitionResultにエラーを入れて即return
// 3. 行が取得できない（EOFなど）時は　まだ行が完成していない->読み込みを一時停止（kSuspend）
// 4. 行が空なら異常->エラーで返す
// 5. 正常に行が読めた時
//    ・TransitionResultに requestLine と status をセットしreturnする

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

// HTTPリクエストのリクエストライン（1行目）を読み取るstateを実装
// 例：GET /index.html HTTP/1.1
// stateパターンの「リクエストライン読み取りフェーズ」を表す

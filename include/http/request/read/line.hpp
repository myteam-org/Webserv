#pragma once

#include "http/request/read/state.hpp"
#include "io/input/read/buffer.hpp"
#include "http/request/read/context.hpp"
#include "http/request/read/reader.hpp"

namespace http {

class ReadingRequestLineState : public IState {
 public:
  ReadingRequestLineState();
  virtual ~ReadingRequestLineState();
  TransitionResult handle(ReadContext& ctx, ReadBuffer& buf);
};

}  // namespace http

// HTTPリクエストのリクエストライン（1行目）を読み取るstateを実装
// stateパターンの「リクエストライン読み取りフェーズ」を表す

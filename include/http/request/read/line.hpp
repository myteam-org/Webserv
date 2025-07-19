#pragma once

#include "state.hpp"
#include "buffer.hpp"
#include "context.hpp"
#include "reader.hpp"

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

#pragma once
#include "buffer.hpp"
#include "reader.hpp"
#include "state.hpp"

namespace http {

class ReadingRequestLineState : public IState {
 public:
  ReadingRequestLineState();
  virtual ~ReadingRequestLineState();
  TransitionResult handle(ReadBuffer& buf);
};

}  // namespace http

// HTTPリクエストのリクエストライン（1行目）を読み取るstateを実装
// stateパターンの「リクエストライン読み取りフェーズ」を表す

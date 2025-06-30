#pragma once
#include "buffer.hpp"
#include "reader.hpp"
#include "state.hpp"

namespace http {

class ReadingRequestLineState : public IState {
 public:
  ReadingRequestLineState();
  virtual ~ReadingRequestLineState();
  virtual TransitionResult handle(const ReadBuffer& buf) const;
  virtual TransitionResult handle(ReadBuffer& buf);  // 追加
};

}  // namespace http

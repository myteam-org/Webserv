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

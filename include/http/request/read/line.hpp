#pragma once
#include "buffer.hpp"
#include "reader.hpp"

namespace http {

class ReadingRequestLineState : public IState {
 public:
  ReadingRequestLineState();
  virtual ~ReadingRequestLineState();
  virtual TransitionResult handle(const ReadBuffer& buf) const;
};

}  // namespace http

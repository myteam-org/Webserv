#pragma once
#include "buffer.hpp"
#include "state.hpp"

// class ReadBuffer;

namespace http {

class ReadingRequestHeadersState : public IState {
   public:
    ReadingRequestHeadersState();
    virtual ~ReadingRequestHeadersState();
    TransitionResult handle(ReadBuffer& buf);
};

}  // namespace http

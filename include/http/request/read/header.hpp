#pragma once
#include "buffer.hpp"
#include "state.hpp"
#include "context.hpp"


namespace http {

class ReadingRequestHeadersState : public IState {
   public:
    ReadingRequestHeadersState();
    virtual ~ReadingRequestHeadersState();
    TransitionResult handle(ReadContext& ctx, ReadBuffer& buf);
};

}  // namespace http

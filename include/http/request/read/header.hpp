#pragma once
#include "buffer.hpp"
#include "state.hpp"
#include "context.hpp"


namespace http {

class ReadingRequestHeadersState : public IState {
   public:
    ReadingRequestHeadersState();
    virtual ~ReadingRequestHeadersState();
    static TransitionResult handle(ReadContext& ctx, ReadBuffer& buf);
   private:
    static IState* createBodyReadingState(const RawHeaders& headers, ReadContext& ctx);
};

}  // namespace http

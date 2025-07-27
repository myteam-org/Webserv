#pragma once
#include "buffer.hpp"
#include "context.hpp"
#include "state.hpp"

namespace http {

class ReadingRequestHeadersState : public IState {
   public:
    ReadingRequestHeadersState();
    virtual ~ReadingRequestHeadersState();
    TransitionResult handle(ReadContext& ctx, ReadBuffer& buf);
    static TransitionResult handleHeadersComplete(ReadContext& ctx,
                                                  TransitionResult& tr,
                                                  const RawHeaders& headers);
};

}  // namespace http

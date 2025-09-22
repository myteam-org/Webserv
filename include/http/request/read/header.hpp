#pragma once
#include "io/input/read/buffer.hpp"
#include "http/request/read/context.hpp"
#include "http/request/read/state.hpp"

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

#include "chunked_body.hpp"

#include "state.hpp"
#include "context.hpp"
#include "utils/types/error.hpp"
#include "utils/types/option.hpp"
#include "utils/types/result.hpp"

namespace http {

ReadingRequestBodyChunkedState::ReadingRequestBodyChunkedState()
    : phase_(kReadSize), currentChunkSize_(0), alreadyRead_(0) {}

ReadingRequestBodyChunkedState::~ReadingRequestBodyChunkedState() {}

TransitionResult ReadingRequestBodyChunkedState::handle(ReadContext& ctx,
                                                       ReadBuffer& buf) {
    (void)ctx;
    TransitionResult tr;

    while (true) {
        if (phase_ == kReadSize) {
            const GetLineResult result = getLine(buf);
            if (result.isErr()) {
                return tr.setStatus(types::err(result.unwrapErr())), tr;
            }
            const types::Option<std::string> lineOpt = result.unwrap();
            if (lineOpt.isNone()) {
                return tr.setStatus(types::ok(kSuspend)), tr;
            }
            
        }
    }
}
}  // namespace http

#include "body.hpp"

#include "raw_headers.hpp"
#include "state.hpp"
#include "utils.hpp"
#include "utils/types/error.hpp"
#include "utils/types/option.hpp"
#include "utils/types/result.hpp"

namespace http {

ReadingRequestBodyState::ReadingRequestBodyState(BodyEncodingType type, const BodyLengthConfig& config)
    : activeBodyState_(NULL), type_(type), config_(config) {
    if (type == kContentLength) {
        activeBodyState_ = new ReadingRequestBodyLengthState(config);
    } 
    // else if (type == kChunked) {
    //     activeBodyState_ = new ReadingRequestBodyChunkedState();
    // }
}

ReadingRequestBodyState::~ReadingRequestBodyState() {
    delete activeBodyState_;
}

TransitionResult ReadingRequestBodyState::handle(ReadContext& ctx, ReadBuffer& buf) {
    TransitionResult tr;
    if (activeBodyState_ == NULL) {
        tr.setStatus(types::err(error::kIOUnknown));
        return tr;
    }
    return activeBodyState_->handle(ctx, buf);
}

}  // namespace http

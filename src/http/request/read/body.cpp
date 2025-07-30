#include "body.hpp"

#include "length_body.hpp"
#include "raw_headers.hpp"
#include "state.hpp"
#include "utils.hpp"
#include "utils/types/error.hpp"
#include "utils/types/option.hpp"
#include "utils/types/result.hpp"

namespace http {

ReadingRequestBodyState::ReadingRequestBodyState(BodyEncodingType type,
                                                 const BodyLengthConfig& config)
    : activeBodyState_(NULL), type_(type), config_(config) {
    switch (type) {
        case kContentLength:
            activeBodyState_ = new ReadingRequestBodyLengthState(config);
            break;
        case kChunked:
            activeBodyState_ = new ReadingRequestBodyChunkedState();
            break;
        default:
            BodyLengthConfig config0;
            config0.contentLength = 0;
            config0.clientMaxBodySize = config_.clientMaxBodySize;
            activeBodyState_ = new ReadingRequestBodyLengthState(config0); 
            break;
    }
}

ReadingRequestBodyState::~ReadingRequestBodyState() { delete activeBodyState_; }

TransitionResult ReadingRequestBodyState::handle(ReadContext& ctx,
                                                 ReadBuffer& buf) {
    if (activeBodyState_ == NULL) {
        TransitionResult tr;
        tr.setStatus(types::err(error::kIOUnknown));
        return tr;
    }
    return activeBodyState_->handle(ctx, buf);
}

}  // namespace http

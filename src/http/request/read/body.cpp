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

ReadingRequestBodyState::~ReadingRequestBodyState() {}

TransitionResult ReadingRequestBodyState::handle(ReadBuffer& buf) {
    TransitionResult tr;
    RawHeaders headers;

    while (true) {
        const GetLineResult result = getLine(buf);
        if (!result.canUnwrap()) {
            tr.setStatus(types::err(result.unwrapErr()));
            return tr;
        }
        const types::Option<std::string> lineOpt = result.unwrap();
        if (lineOpt.isNone()) {
            tr.setStatus(types::ok(kSuspend));
            return tr;
        }
        const std::string line = lineOpt.unwrap();
        if (line.empty()) {
            tr.setHeaders(types::some(headers));
            tr.setStatus(types::ok(kDone));
            return tr;
        }
        const std::string::size_type colon = line.find(':');
        if (colon == std::string::npos) {
            tr.setStatus(types::err(error::kIOUnknown));
            return tr;
        }
        const std::string key = line.substr(0, colon);
        const std::string value = line.substr(colon + 1);
        headers.insert(std::make_pair(key, value));
    }
}

}  // namespace http

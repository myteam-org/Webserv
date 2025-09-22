#pragma once

#include "http/request/read/body.hpp"
#include "http/request/read/utils.hpp"
#include "io/input/read/buffer.hpp"
#include "http/request/read/state.hpp"

namespace http {

class ReadingRequestBodyLengthState : public IState {
   public:
    explicit ReadingRequestBodyLengthState(const BodyLengthConfig& config);
    virtual ~ReadingRequestBodyLengthState();
    virtual TransitionResult handle(ReadContext& ctx, ReadBuffer& buf);
    
   private:
    std::size_t contentLength_;
    std::size_t clientMaxBodySize_;
    std::size_t alreadyRead_;
    std::string bodyBuffer_;

    static TransitionResult done(const std::string& body);
    static TransitionResult error(TransitionResult& tr, error::AppError err);
    static bool ensureData(ReadBuffer& buf, TransitionResult& tr);
    static TransitionResult suspend(TransitionResult& tr);
};
}  // namespace http

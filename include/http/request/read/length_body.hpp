#pragma once

#include "buffer.hpp"
#include "state.hpp"

namespace http {

struct BodyLengthConfig {
    std::size_t contentLength;
    std::size_t clientMaxBodySize;
};

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

    static TransitionResult done_(const std::string& body);
    static TransitionResult error_(TransitionResult& tr, error::AppError err);
    static bool ensureData_(ReadBuffer& buf, TransitionResult& tr);
    static TransitionResult suspend_(TransitionResult& tr);
};
}  // namespace http

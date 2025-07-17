#pragma once

#include "buffer.hpp"
#include "state.hpp"

namespace http {
class ReadingRequestBodyLengthState : public IState {
   public:
    explicit ReadingRequestBodyLengthState(std::size_t contentLength, std::size_t clientMaxBodySize);
    virtual ~ReadingRequestBodyLengthState();
    virtual TransitionResult handle(ReadBuffer& buf);

   private:
    std::size_t contentLength_;
    std::size_t clientMaxBodySize_;
    std::size_t alreadyRead_;
    std::string bodyBuffer_;
};
}  // namespace http

#pragma once

#include "buffer.hpp"
#include "state.hpp"

namespace http {
class ReadingRequestBodyLengthState : public IState {
   public:
    explicit ReadingRequestBodyLengthState(std::size_t contentLength);
    virtual ~ReadingRequestBodyLengthState();
    virtual TransitionResult handle(ReadBuffer& buf);

   private:
    size_t contentLength_;
    size_t alreadyRead_;
    std::string bodyBuffer_;
};
}  // namespace http

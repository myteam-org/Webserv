#pragma once
#include "state.hpp"
#include "buffer.hpp"

// class ReadBuffer;

namespace http {

class ReadingRequestHeadersState : public IState {
   public:
    ReadingRequestHeadersState();
    virtual ~ReadingRequestHeadersState();
    TransitionResult handle(ReadBuffer& buf);
   private:
	std::vector<std::string> headerLine_;
};
    
} // namespace http

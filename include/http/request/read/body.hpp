#pragma once

#include "io/input/write/buffer.hpp"
#include "http/request/read/state.hpp"
#include "http/request/read/chunked_body.hpp"

namespace http {

enum BodyEncodingType {
    kNone,
    kContentLength,
    kChunked
};

struct BodyLengthConfig {
    std::size_t contentLength;
    std::size_t clientMaxBodySize;
};

class ReadingRequestBodyState : public IState {
   public:
    explicit ReadingRequestBodyState(BodyEncodingType type, const BodyLengthConfig& config);
    virtual ~ReadingRequestBodyState();
    virtual TransitionResult handle(ReadContext& ctx, ReadBuffer& buf);

   private:
    IState* activeBodyState_;     // ★ 本体：Length or Chunked を指す
    http::BodyEncodingType type_; // ★ デバッグやロジック判断に残してOK
    BodyLengthConfig config_;     // ★ LengthState に渡す用の初期値
};
}  // namespace http

#pragma once

#include "buffer.hpp"
#include "state.hpp"
#include "length_body.hpp"
#include "chunked_body.hpp"

namespace http {

enum BodyEncodingType {
    kNone,
    kContentLength,
    kChunked
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
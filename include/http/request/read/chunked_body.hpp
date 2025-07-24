#pragma once

#include "buffer.hpp"
#include "state.hpp"

namespace http {

class ReadingRequestBodyChunkedState : public IState {
   public:
    explicit ReadingRequestBodyChunkedState();
    virtual ~ReadingRequestBodyChunkedState();

    virtual TransitionResult handle(ReadContext& ctx, ReadBuffer& buf);
    TransitionResult handleReadSize(ReadBuffer& buf, TransitionResult& tr);
    TransitionResult handleReadData(ReadBuffer& buf, TransitionResult& tr);
    TransitionResult handleReadTrailer(ReadBuffer& buf, TransitionResult& tr);
    TransitionResult handleDone(TransitionResult& tr);

   private:
    enum Phase { kChunkReadSize, kChunkReadData, kChunkReadTrailer, kChunkDone };

    Phase phase_;
    std::string buffer_;            // 一時的に読み込んだ未処理データ
    std::size_t currentChunkSize_;  // 現在処理中のチャンクサイズ
    std::size_t alreadyRead_;       // 現在のチャンクで読み込んだバイト数
    std::string body_;              // 完成したボディを貯めるバッファ
};
}  // namespace http

#pragma once

#include "buffer.hpp"
#include "state.hpp"

namespace http {

class ReadingRequestBodyChunkedState : public IState {
   public:
    explicit ReadingRequestBodyChunkedState();
    virtual ~ReadingRequestBodyChunkedState();

    virtual TransitionResult handle(ReadContext& ctx, ReadBuffer& buf);
    std::size_t parseHex(const std::string& hex);  // 16進数を変換する

   private:
    enum Phase { kReadSize, kReadData, kReadTrailer, kDone };

    Phase phase_;
    std::string buffer_;            // 一時的に読み込んだ未処理データ
    std::size_t currentChunkSize_;  // 現在処理中のチャンクサイズ
    std::size_t alreadyRead_;       // 現在のチャンクで読み込んだバイト数
    std::string body_;              // 完成したボディを貯めるバッファ
};
}  // namespace http

#include "io/input/write/buffer.hpp"

#include <algorithm>
#include <cstring>

WriteBuffer::WriteBuffer(io::IWriter& writer)
    : buf_(), head_(0), writer_(writer) {
    buf_.reserve(kDefaultBufferSize);
}

void WriteBuffer::append(const std::string& data) {
    if (data.empty()) {
        return;
    }
    const std::size_t oldSize = buf_.size();
    buf_.resize(oldSize + data.size());
    std::memcpy(&buf_[0] + oldSize, data.data(), data.size());
}

types::Result<std::size_t, error::AppError> WriteBuffer::flush() {
    std::size_t total = 0;

    while (head_ < buf_.size()) {
        const char* p = &buf_[0] + head_;
        const std::size_t n = buf_.size() - head_;

        io::IWriter::WriteResult wr = writer_.write(p, n);
        if (!wr.isOk()) {
            // if (wr.unwrapErr().isWouldBlock()) {
            //     return types::ok<std::size_t>(total);  // WouldBlock は「ここまでの total」を成功として返す
            // }
            return types::err<error::AppError>(wr.unwrapErr());
        }
        const std::size_t wrote = wr.unwrap();
        if (wrote == 0) {
            return types::ok<std::size_t>(total);  // 0 は「今は書けない」扱い：ここまでの total を返す
        }

        head_ += wrote;
        total += wrote;

        // 送り切ったらバッファをクリア
        if (head_ == buf_.size()) {
            buf_.clear();
            head_ = 0;
            break;
        }
        // buf_ の送信済みデータ(head_ で判別可)が多い場合で、未送信データが残る場合、buf 配列を一定条件に従って切り詰める。
        // head_ は次回送信するべきインデックスを表す。具体的な切り詰め条件は下記。
        // 送信済みの byte 数が定数 (kCompactMinHeadBytes) を超えている。
        // 送信すべき buffer の全体サイズより、送信済みのサイズ * kCompactWasteRatioが大きい。
        if (head_ > kCompactMinHeadBytes && head_ * kCompactWasteRatio > buf_.size()) {
            compact_();
        }
    }

    return types::ok<std::size_t>(total);
}

bool WriteBuffer::isEmpty() const {
    return head_ >= buf_.size();
}

void WriteBuffer::compact_() {
    const std::size_t remain = buf_.size() - head_;
    if (remain) {
        std::memmove(&buf_[0], &buf_[0] + head_, remain);
    }
    buf_.resize(remain);
    head_ = 0;
}

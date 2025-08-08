#include "write/buffer.hpp"

#include <algorithm>
#include <cstring>

WriteBuffer::WriteBuffer(io::IWriter& writer)
    : buf_(), head_(0), writer_(writer) {
    buf_.reserve(4096);
}

void WriteBuffer::append(const std::string& data) {
    if (data.empty()) return;
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
            if (wr.unwrapErr().isWouldBlock()) {
                return types::ok<std::size_t>(total);
            }
            return types::err<std::size_t>(wr.unwrapErr());
        }
        const std::size_t wrote = wr.value();
        if (wrote == 0) {
            return types::ok<std::size_t>(total);
        }

        head_ += wrote;
        total += wrote;

        // 送り切ったらバッファをクリア
        if (head_ == buf_.size()) {
            buf_.clear();
            head_ = 0;
            break;
        }
        // 空きが大きいときは前詰め
        if (head_ > 8192 && head_ * 2 > buf_.size()) {
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

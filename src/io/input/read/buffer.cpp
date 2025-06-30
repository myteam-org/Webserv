#include "buffer.hpp"
#include "utils/types/try.hpp"
#include <string>
#include <algorithm>

ReadBuffer::ReadBuffer(io::IReader &reader) : reader_(reader) {}

types::Option<std::string> ReadBuffer::consumeUntil(const std::string &delimiter) {
    std::string::size_type pos = std::string::npos;
    if (delimiter.empty()) {
        return types::Option<std::string>(types::None());
    }
    for (;;) {
        const std::string buf_str(buf_.begin(), buf_.end());
        pos = buf_str.find(delimiter);
        if (pos != std::string::npos) {
            const std::string result = buf_str.substr(0, pos + delimiter.size());
            buf_.erase(buf_.begin(), buf_.begin() + static_cast<long>(pos + delimiter.size()));
            return types::Option<std::string>(types::Some<std::string>(result));
        }
        if (reader_.eof()) {
            return types::Option<std::string>(types::None());
        }
        const types::Result<std::size_t, error::AppError> loadResult = this->load();
        if (loadResult.isErr()) {
            return types::Option<std::string>(types::None());
        }
        const std::size_t loaded = loadResult.unwrap();
        if (loaded == 0) {
            return types::Option<std::string>(types::None());
        }
    }
}

std::string ReadBuffer::consume(const std::size_t nbyte) {
    const std::size_t bytesToConsume = std::min(nbyte, buf_.size());
    const std::string consumed(buf_.data(), bytesToConsume);
    buf_.erase(buf_.begin(), buf_.begin() + static_cast<long>(bytesToConsume));
    return consumed;
}

ReadBuffer::LoadResult ReadBuffer::load() {
    if (reader_.eof()) {
        return OK(0ul);
    }

    char tmp[ReadBuffer::kLoadSize];
    const std::size_t bytesRead = TRY(reader_.read(tmp, ReadBuffer::kLoadSize));
    buf_.insert(buf_.end(), tmp, tmp + bytesRead);
    return OK(bytesRead);
}

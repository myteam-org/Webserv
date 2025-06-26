#include "buffer.hpp"
#include "utils/types/try.hpp"

ReadBuffer::ReadBuffer(io::IReader &reader) : reader_(reader) {}

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
    buf_.insert(buf_.begin(), tmp, tmp + bytesRead);
    return OK(bytesRead);
}


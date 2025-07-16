#pragma once

#include <string>
#include <vector>
#include "utils/types/option.hpp"
#include "utils/types/result.hpp"
#include "utils/types/error.hpp"
#include "io/input/reader/reader.hpp"

class ReadBuffer {
public:
    explicit ReadBuffer(io::IReader &reader);

    types::Result<types::Option<std::string>, error::AppError>
    consumeUntil(const std::string &delimiter);

    std::string consume(std::size_t nbyte);

    typedef types::Result<std::size_t, error::AppError> LoadResult;
    LoadResult load();
	size_t size() const;

private:
    std::vector<char> buf_;
    io::IReader &reader_;
    static const std::size_t kLoadSize = 4096;
};

// 低レベルなリーダーIReaderからバッファ付きで文字列を読み取るラッパー
// ・ReadBufferはIReader(例えばFdReader)のラッパー
// ・内部にstd::vector<char> buf_を持ち、読み込み済みのデータを一時保持する

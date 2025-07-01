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

    std::string consume(const std::size_t nbyte);

    typedef types::Result<std::size_t, error::AppError> LoadResult;
    LoadResult load();

private:
    std::vector<char> buf_;
    io::IReader &reader_;
    static const std::size_t kLoadSize = 4096;
};

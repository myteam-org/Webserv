#pragma once

#include "reader.hpp"
#include "result.hpp"
#include "option.hpp"
#include <vector>
#include <string>

class ReadBuffer {
public:
    explicit ReadBuffer(io::IReader &reader);

    types::Option<std::string> consumeUntil(const std::string &delimiter);
    std::string consume(std::size_t nbyte);
    typedef types::Result<std::size_t, error::AppError> LoadResult;
    LoadResult load();

private:
    static const std::size_t kLoadSize = 4096;
    io::IReader &reader_;
    std::vector<char> buf_;
};

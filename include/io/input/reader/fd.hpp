#pragma once

#include "io/input/reader/reader.hpp"
#include <cstddef> // for std::size_t

namespace io {

class FdReader : public IReader {
public:
    explicit FdReader(int fd);
    virtual ~FdReader();

    virtual ReadResult read(char *buf, std::size_t nbyte);
    virtual bool eof();

private:
    int fd_;
    bool eof_;
};

} // namespace io

// IReaderを継承して、ファイルディスクリプタ（int fd）を読み込む実装
// 実際には::read(fd_, buf, nbyte)を使う（低レベルな入出力）

#pragma once

#include "io/input/writer/writer.hpp"
#include <cstddef> // for std::size_t

namespace io {

class FdWriter : public IWriter {
public:
    explicit FdWriter(int fd);
    virtual ~FdWriter();
    virtual WriteResult write(char *buf, std::size_t nbyte);
    WriteResult write(const char *buf, std::size_t nbyte);
private:
    int fd_;
};

} // namespace io

// IReaderを継承して、ファイルディスクリプタ（int fd）を読み込む実装
// 実際には::read(fd_, buf, nbyte)を使う（低レベルな入出力）

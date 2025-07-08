#include "io/input/reader/fd.hpp"
#include "io/input/reader/reader.hpp"
#include "utils/types/result.hpp"
#include <unistd.h>
#include <cstring>

namespace io {

FdReader::FdReader(int fd)
    : fd_(fd), eof_(false) {}

FdReader::~FdReader() {
    if (fd_ >= 0) {
        ::close(fd_);
    }
}

FdReader::ReadResult FdReader::read(char *buf, std::size_t nbyte) {
    if (eof_) {
        return OK(static_cast<std::size_t>(0));
    }

    const ssize_t bytesRead = ::read(fd_, buf, nbyte);
    if (bytesRead < 0) {
        return ERR(error::kIOUnknown);
    }
    if (bytesRead == 0) {
        eof_ = true;  // EOF 到達
        return OK(static_cast<std::size_t>(0));
    }

    return OK(static_cast<std::size_t>(bytesRead));
}

bool FdReader::eof() {
    return eof_;
}

} // namespace io

// ファイルやソケットからバイナリデータを読み込むためのクラス
// IReaderという抽象クラスを継承
// 実態として「ファイルディスクリプタから読み込む」処置を提供する
// read()メソッド
// ・読み込み済み（eof == true）なら0を返す
// ・::read(fd_, buf, nbyte)を使って読み込みする
// ・読み込めなければERR(kIOUnknown)を返す
// ・読み込んだバイト数をResult<std::size_t, AppError>型で返す

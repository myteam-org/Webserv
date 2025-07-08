#pragma once

#include "result.hpp"
#include "error.hpp"

namespace io {
    class IReader {
    public:
        typedef types::Result<std::size_t, error::AppError> ReadResult;

        virtual ~IReader() {}
        
        virtual ReadResult read(char *buf, std::size_t nbyte) = 0;
        virtual bool eof() = 0;
    };
} // namespace io

// IReaderインターフェース
// ・読み込み可能ななにか（ファイル、ソケットなど）を抽象化したインターフェース
// ・read()とeof()を必ず実装させる
// ・read()は最大nbyte分読み、成功か失敗（Result）を返す
// まとめ
//  IReader は「何かを読み取れる」共通インターフェース
//  FdReader はそれを「ファイルディスクリプタで実装したもの」
//  ReadBuffer は「IReader からバッファ付きで便利に読み取るクラス」

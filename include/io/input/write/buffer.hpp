#include <iostream>
#include <vector>
#include "utils.hpp"

class WriteBuffer {
public:
    explicit WriteBuffer(io::IWriter &writer); 

    // 書き込みデータ追加
    void append(const std::string &data);

    // 送信処理: バッファの先頭から送信し、送信済み分を削除
    types::Result<std::size_t, error::AppError> flush();

    bool isEmpty() const;

private:
    std::vector<char> buf_;
    io::IWriter &writer_;
};

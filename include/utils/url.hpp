#pragma once

#include <iostream>

namespace utils {
namespace url {

    // 厳密な %XX デコード（不正なエンコードや危険な制御文字を拒否）
    // 成功: true（out に上書き） / 失敗: false（out は未定義）
    // ‘%’が来たら
    // 1) エスケープシーケンスの妥当性チェック
    // 2) 16進文字を数値に変換
    // 3) バイトに復元
    // 4) ASCII 制御文字かどうか検証
    // 5) 出力に追加
    // 6) '%' 以外はそのまま出力
inline bool decodeStrict(const std::string& src, std::string& out) {
    static const unsigned char kAsciiSpace = 0x20;
    out.clear();
    for (std::size_t i = 0; i < src.size(); ++i) {
        const unsigned char chr = static_cast<unsigned char>(src[i]);
        if (chr == '%') {
            if (i + 2 >= src.size()) {
                return false;
            }
            const unsigned char h = (static_cast<unsigned char>(src[i + 1]));
            const unsigned char l = (static_cast<unsigned char>(src[i + 2]));
            if (!std::isxdigit(h) || !std::isxdigit(l)) {
                return false;
            }
            int high = std::isdigit(h) ? (h - '0') : (std::toupper(h) - 'A' + 10);
            int low = std::isdigit(l) ? (l - '0') : (std::toupper(l) - 'A' + 10); // (2)
            unsigned char decoded = static_cast<unsigned char>((high << 4) | low); // (3)
            if (decoded < kAsciiSpace && decoded != '\t') {
                return false; // (4)
            }
            out.push_back(static_cast<char>(decoded)); // (5)
            i += 2;
        } else {
            out.push_back(static_cast<char>(chr)); // (6)
        }
    }
    return true;
}

} // namespace url
} // namespace utils

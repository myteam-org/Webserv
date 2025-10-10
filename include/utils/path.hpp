#pragma once

#include <iostream>
#include <sstream>
#include <vector>
#include "utils/string.hpp"

namespace utils {
namespace path {

inline void compressSegmentCore(const std::string& inString,
                                std::vector<std::string>& segs);

// 共通コア：ドットセグメント削除＆連続 '/' 圧縮
//  - keepAbsolute: 入力が絶対パス（先頭 '/'）なら出力でも保持
//  - keepTrailingSlash: 入力が末尾 '/' なら可能なら保持
inline std::string compressSegments(const std::string& inString,
                                    bool keepAbsolute, bool keepTrailingSlash) {
    const bool isAbs = !inString.empty() && inString[0] == '/';
    const bool hadTrailing =
        !inString.empty() && inString[inString.size() - 1] == '/';

    std::vector<std::string> segs;
    // 圧縮
    compressSegmentCore(inString, segs);
    // 再構築
    std::string out;
    if (keepAbsolute && isAbs) {
        out.push_back('/');
    }
    for (std::size_t i = 0; i < segs.size(); ++i) {
        out += segs[i];
        if (i + 1 != segs.size()) {
            out.push_back('/');
        }
    }
    const bool needKeepTrailing = keepTrailingSlash && hadTrailing;
    const bool hasPathContent = (keepAbsolute && isAbs) || !segs.empty();
    if (needKeepTrailing && hasPathContent) {
        if (!out.empty() && out[out.size() - 1] != '/') {
            out.push_back('/');
        }
    }
    if (out.empty() && keepAbsolute && isAbs) {
        out = "/";
    }
    return out;
}

inline void compressSegmentCore(const std::string& inString,
                                std::vector<std::string>& segs) {
    segs.clear();
    const std::string::size_type stringSize = inString.size();
    static const int TEMPORARY_SEGMENT_CAPACITY = 8;
    if (segs.capacity() < static_cast<std::size_t>(TEMPORARY_SEGMENT_CAPACITY)) {
        segs.reserve(TEMPORARY_SEGMENT_CAPACITY);
    }
    std::string::size_type i = 0;
    while (i < stringSize) {
        while (i < stringSize && inString[i] == '/') {
            ++i;
        }
        std::string::size_type j = i;
        while (j < stringSize && inString[j] != '/') {
            ++j;
        }
        if (j > i) {  // セグメントが空でなければ
            const std::string seg = inString.substr(i, j - i);
            if (seg == ".") {
                // ignore
            } else if (seg == "..") {
                if (!segs.empty()) {
                    segs.pop_back();
                } else {
                    // ignore
                }
            } else {
                segs.push_back(seg);
            }
        }
        i = j;
    }
}

// 使い方1
// RFC 3986 §5.2.4 準拠の dot-segment 除去
// URL の相対解決や正規化の一部として使う
// "." / ".." セグメントを削除
// 事前条件: "%2e" 等は decode_strict 済みで '.' にデコードされていること
inline std::string removeDotSegments(const std::string& decodedPath) {
    return compressSegments(decodedPath, /*keepAbsolute=*/true,
                            /*keepTrailingSlash=*/true);
}

// 使い方2
// ファイルシステム用の「スラッシュ正規化」
//  - バックスラッシュはスラッシュに変換
//  - その後に dot-segment 除去＆連続 '/' 圧縮
inline std::string normalizeSlashes(const std::string& pathLike) {
    std::string tmp;
    tmp.reserve(pathLike.size());

    for (std::size_t i = 0; i < pathLike.size(); ++i) {
        char chr = pathLike[i];
        if (chr == '\\') {
            chr = '/';
        }
        tmp.push_back(chr);
    }
    return compressSegments(tmp, /*keepAbsolute=*/true,
                            /*keepTrailingSlash=*/true);
}

inline bool isPathUnderRoot(const std::string& root,
                                        const std::string& path) {
    if (root.empty()) {
        return false;
    }
    return path.compare(0, root.size(), root) == 0 &&
           (path.size() == root.size() || path[root.size()] == '/');
}

inline std::string stripQuery(const std::string& target) {
    const std::string::size_type pos = target.find('?');
    if (pos == std::string::npos) {
        return target;
    } else {
        return target.substr(0, pos);
    }

}

// "/a/b.py" -> ".py", "/a/b" -> "", "/.htaccess" -> ""（隠しファイル扱い）
inline std::string getExtension(const std::string& path) {
    std::string newPath = stripQuery(path);
    const std::string::size_type slash = newPath.find_last_of('/');
    const std::string::size_type dot = newPath.find_last_of('.');
    if (dot == std::string::npos) {
        return "";
    }
    if (slash != std::string::npos && dot < slash) {
        return "";
    }
    // 先頭ドットだけの隠しファイルは拡張子なしとみなす
    if (dot == 0) {
        return "";
    }
    std::string ext = newPath.substr(dot);
    return utils::toLower(ext);
}

}  // namespace path
}  // namespace utils

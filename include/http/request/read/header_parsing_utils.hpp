#pragma once

#include <string>
#include "raw_headers.hpp"

namespace http {
namespace parser {
	// 汎用ヘッダー取得関数
    std::string extractHeader(const RawHeaders& headers, const std::string& key);
	// host取得関数
    std::string extractHost(const RawHeaders& headers);
	// uri取得関数
    std::string extractUri(const std::string& requestLine);

} // namespase parser
} // namespase http

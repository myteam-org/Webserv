#pragma once

#include <string>
#include "body.hpp"
#include "raw_headers.hpp"

namespace http {
namespace parser {
	// 汎用ヘッダー取得関数
    std::string extractHeader(const RawHeaders& headers, const std::string& key);
	// host取得関数
    std::string extractHost(const RawHeaders& headers);
	// uri取得関数
    std::string extractUri(const std::string& requestLine);
	// ボディがあるかどうか
	bool hasBody(const RawHeaders& headers);
	//"Content-Length"があればkContentLengthを返す
	//"Transfer-Encoding: chunk"をチェックしてkChunkedを返す
	http::BodyEncodingType detectEncoding(const RawHeaders& headers);
	//get Content-Len
	static const int NUMBER = 10;
	std::size_t extractContentLength(const RawHeaders& headers);


} // namespace parser
} // namespace http

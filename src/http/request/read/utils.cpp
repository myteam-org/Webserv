#include "http/request/read/utils.hpp"
#include <string>
#include <algorithm>

namespace {

bool endsWith(const std::string& targetString, const std::string& suffixString) {
    if (suffixString.size() > targetString.size()) {
        return false;
    }
    return std::equal(suffixString.rbegin(), suffixString.rend(), targetString.rbegin());
}

} // anonymous namespace

namespace http {

GetLineResult getLine(ReadBuffer& readBuffer) {
	const types::Option<std::string> maybeLine = readBuffer.consumeUntil("\r\n");
	if (maybeLine.isNone()) {
		return types::ok(types::Option<std::string>(types::None()));
	}
	const std::string line = maybeLine.unwrap();
	if (line.size() < 2) {
		return types::ok(types::Option<std::string>(types::None()));
	}
	if (!endsWith(line, "\r\n")) {
		return types::ok(types::Option<std::string>(types::None()));
	}
	const std::string result = line.substr(0, line.size() - 2);
	return types::ok(types::Option<std::string>(types::Some<std::string>(result)));
}

} // namespace http

#include "utils.hpp"
#include "option.hpp"
#include "result.hpp"
namespace {
	 bool endsWith(const std::string& str, const std::string& suffix) {
		if (suffix.size() > str.size()) {
			return false;
		}
		return std::equal(suffix.rbegin(), suffix.rend(), str.rbegin());
	}
}

namespace http {

GetLineResult getLine(ReadBuffer& readBuf) {
    const types::Option<std::string> maybeLine = readBuf.consumeUntil("\r\n");
    if (maybeLine.isNone()) {
        return types::ok(types::Option<std::string>(types::None()));
    }

    std::string line = maybeLine.unwrap();
    if (!endsWith(line, "\r\n")) {
        return types::err(error::kIOUnknown);
    }

    line.erase(line.size() - 2);
    return types::ok(types::Option<std::string>(types::Some<std::string>(line)));
}

} // namespace http

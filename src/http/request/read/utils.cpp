#include "http/request/read/utils.hpp"
#include <string>
#include <algorithm>

namespace {
    const std::string CRLF = "\r\n";
    const std::size_t CRLF_SIZE = CRLF.size();

    bool endsWith(const std::string& targetString, const std::string& suffixString) {
        if (suffixString.size() > targetString.size()) {
            return false;
        }
        return std::equal(suffixString.rbegin(), suffixString.rend(), targetString.rbegin());
    }
} // anonymous namespace

namespace http {
GetLineResult getLine(ReadBuffer& readBuffer) {
    const types::Result<types::Option<std::string>, error::AppError> maybeLineResult = readBuffer.consumeUntil(CRLF);
    if (maybeLineResult.isErr()) {
        return types::err(maybeLineResult.unwrapErr());
    }
    const types::Option<std::string> maybeLine = maybeLineResult.unwrap();
    if (maybeLine.isNone()) {
        return types::ok(types::none<std::string>());
    }
    const std::string line = maybeLine.unwrap();
    if (line.size() < CRLF_SIZE) {
        return types::ok(types::none<std::string>());
    }
    if (!endsWith(line, CRLF)) {
        return types::ok(types::none<std::string>());
    }
    const std::string result = line.substr(0, line.size() - CRLF_SIZE);
    return types::ok(types::some<std::string>(result));
}
} // namespace http

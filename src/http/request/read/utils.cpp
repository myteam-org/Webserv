#include "utils.hpp"
#include "option.hpp"
#include "result.hpp"
http::GetLineResult http::getLine(ReadBuffer &readBuf) {
    const types::Option<std::string> maybeLine = readBuf.consumeUntil("\r\n");
    if (maybeLine.isNone()) {
        return types::Ok(types::make);
    }

    std::string line = maybeLine.unwrap();
    if (!utils::endsWith(line, "\r\n")) {
        return err(error::kParseUnknown);
    }

    line.erase(line.size() - 2);
    return types::ok(types::Some(line));
}

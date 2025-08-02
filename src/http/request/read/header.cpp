#include "header.hpp"

#include "body.hpp"
#include "config/context/serverContext.hpp"
#include "context.hpp"
#include "header_parsing_utils.hpp"
#include "http/config/config_resolver.hpp"
#include "length_body.hpp"
#include "raw_headers.hpp"
#include "state.hpp"
#include "utils.hpp"
#include "utils/string.hpp"
#include "utils/types/error.hpp"
#include "utils/types/option.hpp"
#include "utils/types/result.hpp"

namespace http {

ReadingRequestHeadersState::ReadingRequestHeadersState() {}
ReadingRequestHeadersState::~ReadingRequestHeadersState() {}

// handleの目的
// 1. HTTPヘッダーを読み込む
// 2. Content-LengthやTransfer-Encodingを見て、Bodyが必要か判断する
// 3. 必要なら次の状態(ReadingRequestBodyState)へ遷移させる
// 4. TransitionResultにヘッダーや次のステートをセットして返す

TransitionResult ReadingRequestHeadersState::handle(ReadContext& ctx,
                                                    ReadBuffer& buf) {
    TransitionResult tr;
    RawHeaders headers;

    while (true) {
        const GetLineResult result = getLine(buf);
        if (result.isErr()) {
            return tr.setStatus(types::err(result.unwrapErr())), tr;
        }
        const types::Option<std::string> lineOpt = result.unwrap();
        if (lineOpt.isNone()) {
            return tr.setStatus(types::ok(kSuspend)), tr;
        }
        const std::string line = lineOpt.unwrap();
        if (!line.empty() && (line[0] == ' ' || line[0] == '\t')) {
            return tr.setStatus(types::err(error::kBadRequest)), tr;
        }
        if (line.empty()) {
            return handleHeadersComplete(ctx, tr, headers);
        }
        const std::string::size_type colon = line.find(':');
        if (colon == std::string::npos || colon == 0 ||
            std::isspace(line[colon - 1])) {
            tr.setStatus(types::err(error::kBadRequest));
            return tr;
        }
        const std::string key = utils::toLower(utils::trim(line.substr(0, colon)));
        const std::string value = utils::trim(line.substr(colon + 1));
        headers.insert(std::make_pair(key, value));
    }
}

TransitionResult ReadingRequestHeadersState::handleHeadersComplete(
    ReadContext& ctx, TransitionResult& tr, const RawHeaders& headers) {
    const std::string host = parser::extractHeader(headers, "Host");
    const types::Result<const ServerContext*, error::AppError> result =
        ctx.getConfigResolver().chooseServer(host);
    if (result.isErr()) {
        tr.setStatus(types::err(result.unwrapErr()));
        return tr;
    }
    const ServerContext* serverContext = result.unwrap();
    if (serverContext == NULL) {
        tr.setStatus(types::err(error::kIOUnknown));
        return tr;
    }
    ctx.setServer(*serverContext);
    tr.setHeaders(types::some(headers));
    tr.setStatus(types::ok(kDone));
    return tr;
}

}  // namespace http

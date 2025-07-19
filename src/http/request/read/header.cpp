#include "header.hpp"

#include "raw_headers.hpp"
#include "state.hpp"
#include "body.hpp"
#include "context.hpp"
#include "utils.hpp"
#include "header_parsing_utils.hpp"
#include "utils/types/error.hpp"
#include "utils/types/option.hpp"
#include "utils/types/result.hpp"
#include "config/context/serverContext.hpp"

namespace http {

ReadingRequestHeadersState::ReadingRequestHeadersState() {}
ReadingRequestHeadersState::~ReadingRequestHeadersState() {}

// handleの目的
// 1. HTTPヘッダーを読み込む
// 2. Content-LengthやTransfer-Encodingを見て、Bodyが必要か判断する
// 3. 必要なら次の状態(ReadingRequestBodyState)へ遷移させる
// 4. TransitionResultにヘッダーや次のステートをセットして返す

TransitionResult ReadingRequestHeadersState::handle(ReadContext& ctx, ReadBuffer& buf) {
    TransitionResult tr;
    RawHeaders headers;

    while (true) {
        const GetLineResult result = getLine(buf);
        if (!result.canUnwrap()) {
            return tr.setStatus(types::err(result.unwrapErr())), tr;
        }
        const types::Option<std::string> lineOpt = result.unwrap();
        if (lineOpt.isNone()) {
            return tr.setStatus(types::ok(kSuspend)), tr;
        }
        const std::string line = lineOpt.unwrap();
        if (line.empty()) {
            tr.setHeaders(types::some(headers));//読み取ったヘッダーをセットする
            tr.setStatus(types::ok(kDone));
			if (parser::hasBody(headers)) {
                tr.setNextState(createBodyReadingState(headers, ctx));
            }
            return tr;
        }
        const std::string::size_type colon = line.find(':');
        if (colon == std::string::npos) {
            return tr.setStatus(types::err(error::kIOUnknown)), tr;
        }
        const std::string key = line.substr(0, colon);
        const std::string value = line.substr(colon + 1);
        headers.insert(std::make_pair(key, value));
    }
}

IState* ReadingRequestHeadersState::createBodyReadingState(const RawHeaders& headers, ReadContext& ctx) {
    BodyEncodingType type = parser::detectEncoding(headers);
    std::string host = parser::extractHeader(headers, "Host");
    const ServerContext& serverConfig = ctx.getConfigResolver().choseServer(host);
    BodyLengthConfig config;
    config.contentLength = serverConfig.getClientMaxBodySize();
    config.clientMaxBodySize = serverConfig.getClientMaxBodySize();
    return new ReadingRequestBodyState(type, config);
}


}  // namespace http

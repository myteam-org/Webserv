#include "http/request/read/reader.hpp"

// c++98
#include "http/request/read/state.hpp"
#include "http/request/read/line.hpp"
#include "http/request/read/header.hpp"
#include "http/request/read/body.hpp"
#include "http/request/parse/request_parser.hpp"
#include "http/request/request.hpp"
#include "utils/types/try.hpp"
#include "utils/types/result.hpp"
#include "utils/types/option.hpp"
#include "utils/types/error.hpp"
#include "utils/logger.hpp"

namespace http {

RequestReader::RequestReader(config::IConfigResolver& resolver)
    : ctx_(resolver, new ReadingRequestLineState()),
      parser_(types::none<parse::RequestParser*>()),
      headersParsed_(false),
      bodyParsed_(false) {}

RequestReader::~RequestReader() {
    destroyParser_();
}

void RequestReader::destroyParser_() {
    if (parser_.canUnwrap()) {
        parse::RequestParser* p = parser_.unwrap();
        delete p;
        parser_ = types::none<parse::RequestParser*>();
    }
}

types::Result<types::Unit, error::AppError>
RequestReader::ensureParserInited_() {
    if (!parser_.canUnwrap()) {
        parse::RequestParser* p = new parse::RequestParser(ctx_);
        parser_ = types::some(p);
    }
    return types::ok(types::Unit());
}

types::Result<types::Unit, error::AppError>
RequestReader::parseHeadersIfNeeded_() {
    if (headersParsed_) {
        return types::ok(types::Unit());
    }
    if (!ctx_.isHeadersComplete()) {
        return types::ok(types::Unit());
    }
    TRY(ensureParserInited_());
    parse::RequestParser* p = parser_.unwrap();
    TRY(p->parseRequestLine());
    TRY(p->parseHeaders());
    headersParsed_ = true;
    return types::ok(types::Unit());
}

types::Result<types::Unit, error::AppError>
RequestReader::parseBodyIfNeeded_() {
    if (bodyParsed_) {
        return types::ok(types::Unit());
    }
    if (!ctx_.isBodyComplete()) {
        return types::ok(types::Unit());
    }
    TRY(ensureParserInited_());
    parse::RequestParser* p = parser_.unwrap();
    TRY(p->parseBody());
    bodyParsed_ = true;
    return types::ok(types::Unit());
}

types::Result<Request, error::AppError>
RequestReader::buildIfReady_() {
    if (!headersParsed_ || !bodyParsed_) {
        return types::err(error::kBadRequest);
    }
    if (!parser_.canUnwrap()) {
        return types::err(error::kBadRequest);
    }
    parse::RequestParser* p = parser_.unwrap();
    types::Result<Request, error::AppError> built = p->buildRequest();

    // 次リクエスト準備
    destroyParser_();
    headersParsed_ = false;
    bodyParsed_ = false;
    ctx_.resetForNext();
    return built;
}

RequestReader::ReadRequestResult
RequestReader::readRequest(ReadBuffer& buf) {
    // state 完了後の残処理パス
    if (ctx_.getState() == NULL) {
        TRY(parseHeadersIfNeeded_());
        TRY(parseBodyIfNeeded_());
        if (headersParsed_ && bodyParsed_) {
            return types::ok(types::some(TRY(buildIfReady_())));
        }
        return types::ok(types::none<Request>());
    }

    for (;;) {
        const types::Result<IState::HandleStatus, error::AppError> r =
            ctx_.handle(buf);
        if (r.isErr()) {
            // エラー時パーサ破棄
            destroyParser_();
            return types::err(r.unwrapErr());
        }

        if (ctx_.isHeadersComplete() && !headersParsed_) {
            const types::Result<types::Unit, error::AppError> ph =
                parseHeadersIfNeeded_();
            if (ph.isErr()) {
                destroyParser_();
                return types::err(ph.unwrapErr());
            }
        }

        if (ctx_.getState() == NULL) {
            const types::Result<types::Unit, error::AppError> pb =
                parseBodyIfNeeded_();
            if (pb.isErr()) {
                destroyParser_();
                return types::err(pb.unwrapErr());
            }
            if (headersParsed_ && bodyParsed_) {
                return types::ok(types::some(TRY(buildIfReady_())));
            }
            return types::ok(types::none<Request>());
        }

        if (r.unwrap() == IState::kSuspend) {
            return types::ok(types::none<Request>());
        }
        // kDone -> 次 state へ進むのでループ継続
    }
}

} // namespace http

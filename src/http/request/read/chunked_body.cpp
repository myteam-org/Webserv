#include "chunked_body.hpp"

#include "buffer.hpp"
#include "context.hpp"
#include "state.hpp"
#include "utils.hpp"
#include "utils/string.hpp"
#include "utils/types/error.hpp"
#include "utils/types/option.hpp"
#include "utils/types/result.hpp"

namespace http {

ReadingRequestBodyChunkedState::ReadingRequestBodyChunkedState()
    : phase_(kChunkReadSize), currentChunkSize_(0), alreadyRead_(0) {}

ReadingRequestBodyChunkedState::~ReadingRequestBodyChunkedState() {}

TransitionResult ReadingRequestBodyChunkedState::handle(ReadContext& ctx,
                                                        ReadBuffer& buf) {
    TransitionResult tr;

    while (true) {
        switch (phase_) {
            case kChunkReadSize:
                return handleReadSize(buf, tr);
            case kChunkReadData:
                return handleReadData(buf, tr);
            case kChunkReadTrailer:
                return handleReadTrailer(buf, tr);
            case kChunkDone:
                return handleDone(ctx, tr);
        }
    }
    tr.setStatus(types::err(error::kIOUnknown));
    return tr;
}

TransitionResult ReadingRequestBodyChunkedState::handleReadSize(
    ReadBuffer& buf, TransitionResult& tr) {
    const GetLineResult result = getLine(buf);

    if (result.isErr()) {
        tr.setStatus(types::err(result.unwrapErr()));
        return tr;
    }

    const types::Option<std::string> lineOpt = result.unwrap();

    if (lineOpt.isNone()) {
        tr.setStatus(types::ok(kSuspend));
        return tr;
    }

    const std::string line = lineOpt.unwrap();
    const types::Result<std::size_t, error::AppError> sizeResult = utils::parseHex(line);
    if (sizeResult.isErr()) {
        return tr.setStatus(types::err(sizeResult.unwrapErr())), tr;
    }
    currentChunkSize_ = sizeResult.unwrap();
    alreadyRead_ = 0;

    if (currentChunkSize_ == 0) {
        phase_ = kChunkReadTrailer;
    } else {
        phase_ = kChunkReadData;
    }

    tr.setStatus(types::ok(kSuspend));
    return tr;
}

TransitionResult ReadingRequestBodyChunkedState::handleReadData(
    ReadBuffer& buf, TransitionResult& tr) {
    const std::size_t remain = currentChunkSize_ - alreadyRead_;
    const std::size_t toRead = std::min(remain, buf.size());

    if (toRead == 0) {
        const ReadBuffer::LoadResult loadResult = buf.load();
        if (loadResult.isErr()) {
            return tr.setStatus(types::err(loadResult.unwrapErr())), tr;
        }
        return tr.setStatus(types::ok(kSuspend)), tr;
    }

    const std::string chunk = buf.consume(toRead);
    body_ += chunk;
    alreadyRead_ += chunk.size();
    if (alreadyRead_ >= currentChunkSize_) {
        const GetLineResult crlf = getLine(buf);
        if (crlf.isErr()) {
            return tr.setStatus(types::err(crlf.unwrapErr())), tr;
        }
        const types::Option<std::string> lineOpt = crlf.unwrap();
        if (lineOpt.isNone()) {
            return tr.setStatus(types::ok(kSuspend)), tr;
        }
        const std::string& line = lineOpt.unwrap();
        if (!line.empty()) {
            return tr.setStatus(types::err(error::kBadRequest)), tr;
        }
        phase_ = kChunkReadSize;
    }
    return tr.setStatus(types::ok(kSuspend)), tr;
}

TransitionResult ReadingRequestBodyChunkedState::handleReadTrailer(
    ReadBuffer& buf, TransitionResult& tr) {
    const GetLineResult result = getLine(buf);

    if (result.isErr()) {
        tr.setStatus(types::err(result.unwrapErr()));
        return tr;
    }

    const types::Option<std::string> lineOpt = result.unwrap();

    if (lineOpt.isNone()) {
        tr.setStatus(types::ok(kSuspend));
        return tr;
    }

    const std::string line = lineOpt.unwrap();

    if (line.empty()) {
        phase_ = kChunkDone;
    }

    tr.setStatus(types::ok(kSuspend));
    return tr;
}

TransitionResult ReadingRequestBodyChunkedState::handleDone(
    ReadContext& ctx, TransitionResult& tr) {
    ctx.setBody(body_);
    tr.setBody(types::some(body_));
    tr.setStatus(types::ok(kDone));
    return tr;
}

}  // namespace http

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

    switch (phase_) {
        case kChunkReadSize:
            return handleReadSize(buf, tr);
        case kChunkReadData:
            return handleReadData(buf, tr);
        case kChunkReadTrailer:
            return handleReadTrailer(buf, tr, ctx);
        case kChunkDone:
            return handleDone(ctx, tr);
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
    std::string chunkSizePart = line.substr(0, line.find(';'));
    chunkSizePart = utils::trim(chunkSizePart);
    if (chunkSizePart.empty()) {
        return tr.setStatus(types::err(error::kBadRequest)), tr;
    }
    const types::Result<std::size_t, error::AppError> sizeResult =
        utils::parseHex(chunkSizePart);
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
    if (!tryLoadBufferIfEmpty(buf, tr)) {
        return tr;
    }
    const std::size_t remain = currentChunkSize_ - alreadyRead_;
    const std::size_t toRead = std::min(remain, buf.size());
    if (toRead == 0) {
        return tr.setStatus(types::ok(kSuspend)), tr;
    }
    const std::string chunk = buf.consume(toRead);
    body_ += chunk;
    alreadyRead_ += chunk.size();
    if (alreadyRead_ == currentChunkSize_) {
        if (!handleReadCRLFIfDone(buf, tr)) {
            return tr;
        }    
    }
    return tr.setStatus(types::ok(kSuspend)), tr;
}

bool ReadingRequestBodyChunkedState::tryLoadBufferIfEmpty(
    ReadBuffer& buf, TransitionResult& tr) const {
    const std::size_t remain = currentChunkSize_ - alreadyRead_;
    if (std::min(remain, buf.size()) != 0) {
        return true;
    }
    const ReadBuffer::LoadResult loadResult = buf.load();
    if (loadResult.isErr()) {
        tr.setStatus(types::err(loadResult.unwrapErr()));
        return false;
    }
    return true;
}

bool ReadingRequestBodyChunkedState::handleReadCRLFIfDone(
    ReadBuffer& buf, TransitionResult& tr) {
    const GetLineResult crlf = getLine(buf);
    if (crlf.isErr()) {
        tr.setStatus(types::err(crlf.unwrapErr()));
        return false;
    }
    const types::Option<std::string> lineOpt = crlf.unwrap();
    if (lineOpt.isNone()) {
        tr.setStatus(types::ok(kSuspend));
        return false;
    }
    const std::string& line = lineOpt.unwrap();
    if (!line.empty()) {
        tr.setStatus(types::err(error::kBadRequest));
        return false;
    }
    phase_ = kChunkReadSize;
    return true;
}

TransitionResult ReadingRequestBodyChunkedState::handleReadTrailer(
    ReadBuffer& buf, TransitionResult& tr, ReadContext& ctx) {
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
        return handleDone(ctx, tr);
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

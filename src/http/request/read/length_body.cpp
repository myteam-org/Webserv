#include "http/request/read/length_body.hpp"

#include "io/input/read/buffer.hpp"
#include "http/request/read/state.hpp"
#include "utils/types/error.hpp"
#include "utils/types/option.hpp"
#include "utils/types/result.hpp"

namespace http {

ReadingRequestBodyLengthState::ReadingRequestBodyLengthState(
    const BodyLengthConfig& config)
    : contentLength_(config.contentLength),
      clientMaxBodySize_(config.clientMaxBodySize),
      alreadyRead_(0) {}

ReadingRequestBodyLengthState::~ReadingRequestBodyLengthState() {}

// Content-Lengthで指定されたbodyサイズ分だけbufから読み取りためておく
// 全部読み取れないなどのエラーはソケット側でタイムアウト処理をするのでここでは感知しない
TransitionResult ReadingRequestBodyLengthState::handle(ReadContext& ctx,
                                                       ReadBuffer& buf) {
    (void)ctx;
    TransitionResult tr;

    if (contentLength_ == 0) {
        return done(std::string(""));
    }
    if (contentLength_ > clientMaxBodySize_) {
        buf.clear();
        return error(tr, error::kRequestEntityTooLarge);
    }
    if (!ensureData(buf, tr)) {
        return tr;
    }

    const std::size_t remain = contentLength_ - alreadyRead_;
    const std::size_t toRead = std::min(remain, buf.size());

    if (toRead == 0) {
        return suspend(tr);
    }

    const std::string segment = buf.consume(toRead);  // 読み取って消費
    bodyBuffer_ += segment;
    alreadyRead_ += segment.size();
    if (alreadyRead_ >= contentLength_) {
        tr.setBody(types::some(bodyBuffer_));
        tr.setStatus(types::ok(IState::kDone));
        return tr;
    }
    tr.setStatus(types::ok(IState::kSuspend));
    return tr;
}

TransitionResult ReadingRequestBodyLengthState::done(const std::string& body) {
    TransitionResult tr;

    tr.setBody(types::some(body));
    tr.setStatus(types::ok(IState::kDone));
    return tr;
}

TransitionResult ReadingRequestBodyLengthState::error(TransitionResult& tr, error::AppError err) {
    tr.setStatus(types::err(err));
    return tr;
}

bool ReadingRequestBodyLengthState::ensureData(ReadBuffer& buf, TransitionResult& tr) {
    if (buf.size() > 0) {
        return true;
    }
    const ReadBuffer::LoadResult loadResult = buf.load();
    if (loadResult.isErr()) {
        tr.setStatus(types::err(loadResult.unwrapErr()));
        return false;
    }
    if (loadResult.unwrap() == 0) {
        tr.setStatus(types::ok(IState::kSuspend));
        return false;
    }
    return true;
}

TransitionResult ReadingRequestBodyLengthState::suspend(TransitionResult& tr) {
    tr.setStatus(types::ok(IState::kSuspend));
    return tr;
}

}  // namespace http

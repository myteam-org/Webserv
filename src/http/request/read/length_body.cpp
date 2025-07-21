#include "length_body.hpp"

#include "state.hpp"
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
    const ReadBuffer::LoadResult loadResult = buf.load();
    if (loadResult.isErr()) {
        return tr.setStatus(types::err(loadResult.unwrapErr())), tr;
    }
    const std::size_t loaded = loadResult.unwrap();
    if (alreadyRead_ == 0 && contentLength_ > clientMaxBodySize_) {
        return tr.setStatus(types::err(error::kRequestEntityTooLarge)), tr;
    }

    const std::size_t remain = contentLength_ - alreadyRead_;
    const std::size_t toRead = std::min(remain, buf.size());

    if (toRead == 0) {
        return tr.setStatus(types::ok(IState::kSuspend)), tr;  // データ待ち
    }

    const std::string segment = buf.consume(toRead);  // 読み取って消費
    bodyBuffer_ += segment;
    alreadyRead_ += segment.size();

    if (alreadyRead_ >= contentLength_) {
        tr.setBody(types::some(bodyBuffer_));
        tr.setStatus(types::ok(IState::kDone));
        return tr;
    }
    return tr.setStatus(types::ok(IState::kSuspend)), tr;
}
}  // namespace http

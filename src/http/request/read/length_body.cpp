#include "length_body.hpp"

#include "state.hpp"
#include "utils/types/option.hpp"
#include "utils/types/result.hpp"

namespace http {

ReadingRequestBodyLengthState::ReadingRequestBodyLengthState(
    std::size_t contentLength)
    : contentLength_(contentLength), alreadyRead_(0) {}

ReadingRequestBodyLengthState::~ReadingRequestBodyLengthState() {}

// Content-Lengthで指定されたbodyサイズ分だけbufから読み取りためておく
// 全部読み取れないなどのエラーはソケット側でタイムアウト処理をするのでここでは感知しない
TransitionResult ReadingRequestBodyLengthState::handle(ReadBuffer& buf) {
    TransitionResult tr;
    const std::size_t remain = contentLength_ - alreadyRead_;
    const std::size_t toRead = std::min(remain, buf.size());

    if (toRead == 0) {
        tr.setStatus(types::ok(IState::kSuspend));  // データ待ち
        return tr;
    }

    const std::string segment = buf.consume(toRead);  // 読み取って消費
    bodyBuffer_ += segment;
    alreadyRead_ += segment.size();

    if (alreadyRead_ >= contentLength_) {
        tr.setBody(types::some(bodyBuffer_));
        tr.setStatus(types::ok(IState::kDone));
    } else {
        tr.setStatus(types::ok(IState::kSuspend));
    }

    return tr;
}
}  // namespace http

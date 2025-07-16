#include "length_body.hpp"
#include "state.hpp"
#include "utils/types/result.hpp"
#include "utils/types/option.hpp"

namespace http {

ReadingRequestBodyLengthState::ReadingRequestBodyLengthState(std::size_t contentLength)
	: contentLength_(contentLength), alreadyRead_(0) {}

ReadingRequestBodyLengthState::~ReadingRequestBodyLengthState() {}

// Content-Lengthで指定されたbodyサイズ分だけbufから読み取りためておく
// 全部読み取れないなどのエラーはソケット側でタイムアウト処理をするのでここでは感知しない
TransitionResult ReadingRequestBodyLengthState::handle(ReadBuffer& buf) {
	TransitionResult tr;
	std::size_t remain = contentLength_ - alreadyRead_;
	std::size_t toRoad = std::min(remain, buf.size());

	if (toRoad == 0) {
		tr.setStatus(types::ok(IState::kSuspend)); // データ待ち
		return tr;
	}

	std::string segment = buf.consume(toRoad); // 読み取って消費
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
} // namespace http

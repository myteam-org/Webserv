#include "header.hpp"
#include "raw_headers.hpp"
#include "state.hpp"
#include "utils.hpp"
#include "utils/types/try.hpp"
#include "utils/types/result.hpp"
#include "utils/types/option.hpp"
#include "utils/types/error.hpp"
#include "line.hpp"

namespace http {

ReadingHeadersState::ReadingHeadersState() {}
ReadingHeadersState::~ReadingHeadersState() {}

TransitionResult ReadingHeadersState::handle(ReadBuffer& buf) {
	TransitionResult tr;
	RawHeaders headers;

	while (true) {
		const GetLineResult result = getLine(buf);
		if (!result.canUnwrap()) {
			tr.setStatus(types::err(result.unwrapErr()));
			return tr;
		}

		const types::Option<std::string> lineOpt = result.unwrap();
		if (lineOpt.isNone()) {
			tr.setStatus(ok(kSuspend));
			return tr;
		}

		const std::string line = lineOpt.unwrap();
		if (line.empty()) {
			tr.setHeaders(some(headers));
			tr.setStatus(ok(kDone));
			return tr;
		}
		
	}
}

}

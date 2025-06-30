#include "reader.hpp"
#include "state.hpp"
#include "context.hpp"
#include "line.hpp"
#include "utils/types/result.hpp"
#include "utils/types/option.hpp"
#include "utils/types/error.hpp"
#include "io/input/read/buffer.hpp"
#include "config/config.hpp"
#include "header.hpp"
#include "body.hpp"
#include "utils/types/try.hpp"
#include "http/request/request.hpp"  // ← パスを修正！

namespace http {

RequestReader::RequestReader(IConfigResolver& resolver)
    : ctx_(resolver, new ReadingRequestLineState()) {
}

RequestReader::ReadRequestResult RequestReader::readRequest(
    ReadBuffer& buf) {

  std::size_t loaded = TRY(buf.load());

  while (ctx_.getState() != NULL) {
    types::Result<IState::HandleStatus, error::AppError> r = ctx_.handle(buf);

    if (r.isErr()) {
      return types::Result<types::Option<Request>, error::AppError>(types::err(r.unwrapErr()));
    }
    if (r.unwrap() == IState::kSuspend) {
      if (loaded == 0) {
        return types::Result<types::Option<Request>, error::AppError>(types::err(error::kIOUnknown));
      }
      return types::Result<types::Option<Request>, error::AppError>(types::ok(types::Option<Request>(types::None())));
    }
  }

  // Request req = TRY(RequestParser::parseRequest(
  //     ctx_.getRequestLine(), ctx_.getHeaders(), ctx_.getBody()));
  // return types::Result<types::Option<Request>, error::AppError>(types::ok(types::Some(req)));
  return types::Result<types::Option<Request>, error::AppError>(types::ok(types::Option<Request>(types::None())));
}

}  // namespace http

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
    // r → handleResult にリネーム
    types::Result<IState::HandleStatus, error::AppError> handleResult =
        ctx_.handle(buf);

    if (handleResult.isErr()) {
      return types::Result<types::Option<Request>, error::AppError>(
          types::err(handleResult.unwrapErr()));
    }

    if (handleResult.unwrap() == IState::kSuspend) {
      if (loaded == 0) {
        return types::Result<types::Option<Request>, error::AppError>(
            types::err(error::kIOUnknown));
      }
      return types::Result<types::Option<Request>, error::AppError>(
          types::ok(types::Option<Request>(types::None())));
    }
  }

  return types::Result<types::Option<Request>, error::AppError>(
      types::ok(types::Option<Request>(types::None())));
}

}  // namespace http


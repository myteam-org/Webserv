#pragma once

#include "utils.hpp"
#include "context.hpp"

namespace http {

class IConfigResolver;
class Request;

class RequestReader {
 public:
  explicit RequestReader(IConfigResolver& resolver);

  typedef types::Result<types::Option<Request>, error::AppError>
      ReadRequestResult;

  ReadRequestResult readRequest(ReadBuffer& buf);

 private:
  ReadContext ctx_;
};

}  // namespace http


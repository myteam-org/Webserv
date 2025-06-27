namespace http {

TransitionResult::TransitionResult()
    : next_state(NULL),
      request_line(None),
      headers(None),
      body(None),
      status(Result<IState::HandleStatus, error::AppError>::ok(
          IState::kSuspend)) {
}

IState::~IState() {
}

RequestReader::RequestReader(IConfigResolver& resolver)
    : ctx_(resolver, new ReadingRequestLineState()) {
}

RequestReader::ReadRequestResult RequestReader::readRequest(
    ReadBuffer& buf) {

  std::size_t loaded = TRY(buf.load());

  while (ctx_.getState() != NULL) {
    Result<IState::HandleStatus, error::AppError> r = ctx_.handle(buf);

    if (r.isErr()) {
      return Result<Option<Request>, error::AppError>::err(r.unwrapErr());
    }
    if (r.unwrap() == IState::kSuspend) {
      if (loaded == 0) {
        return Result<Option<Request>, error::AppError>::err(error::kUnknown);
      }
      return Result<Option<Request>, error::AppError>::ok(None);
    }
  }

  Request req = TRY(RequestParser::parseRequest(
		ctx_.getRequestLine(), ctx_.getHeaders(), ctx_.getBody()));
  return Result<Option<Request>, error::AppError>::ok(Some(req));
}

}  // namespace http


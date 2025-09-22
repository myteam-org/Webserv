#include "http/handler/file/redirect.hpp"
#include "http/response/builder.hpp"

namespace http {
    RedirectHandler::RedirectHandler(const std::string &destination) : destination_(destination) {}

    Either<IAction *, Response> RedirectHandler::serve(const Request &request) {
        return Right(this->serveInternal(request));
    }

    Response RedirectHandler::serveInternal(const Request &request) const {
        (void) request;
        return ResponseBuilder().status(kStatusFound).header("Location", destination_).build();
    }
} //namespace http

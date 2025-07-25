#include "delete.hpp"
#include "http/response/builder.hpp"
#include "utils/logger.hpp"
#include <sys/stat.h>

namespace http {
    DeleteFileHandler::DeleteFileHandler(const DocumentRootConfig &docRootConfig)
        : docRootConfig_(docRootConfig) {}

    Either<IAction *, Response> DeleteFileHandler::serve(const Request &request) {
        return Right(this->serveInternal(request));
    }

    Response DeleteFileHandler::serveInternal(const Request &req) const {
        const std::string path = docRootConfig_.getRoot() + '/' + req.getRequestTarget();

        struct stat buf = {};
        if (stat(path.c_str(), &buf) == -1) {
            if (errno == ENOENT) {
                return ResponseBuilder().status(kStatusNotFound).build();
            }
            return ResponseBuilder().status(kStatusInternalServerError).build();
        }

        if (!S_ISREG(buf.st_mode)) {
            return ResponseBuilder().status(kStatusForbidden).build();
        }

        if (std::remove(path.c_str())) {
            return ResponseBuilder().status(kStatusInternalServerError).build();
        }

        return ResponseBuilder().status(kStatusNoContent).build();
    }
} //namespace http

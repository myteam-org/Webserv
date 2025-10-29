#include "http/handler/file/delete.hpp"
#include "http/response/builder.hpp"
#include "utils/logger.hpp"
#include <sys/stat.h>
#include <errno.h>
#include <cstdio>
#include <string.h>

namespace http {

    DeleteFileHandler::DeleteFileHandler(const DocumentRootConfig &docRootConfig)
        : docRootConfig_(docRootConfig), locationPath_("") {}

    DeleteFileHandler::DeleteFileHandler(const DocumentRootConfig &docRootConfig,
                                         const std::string &locationPath)
        : docRootConfig_(docRootConfig), locationPath_(locationPath)
    {
        if (locationPath_.length() > 1 &&
            locationPath_[locationPath_.length() - 1] == '/')
            locationPath_.erase(locationPath_.length() - 1);
    }

    DeleteFileHandler::~DeleteFileHandler() {
    }

    Either<IAction *, Response> DeleteFileHandler::serve(const Request &request) {
         LOG_INFO("DeleteHandler is invoked");
        return Right(this->serveInternal(request));
    }

    std::string DeleteFileHandler::stripOneLeadingSlash(const std::string &s) {
        if (!s.empty() && s[0] == '/')
            return s.substr(1);
        return s;
    }

    std::string DeleteFileHandler::normalizeRelative(const std::string &s) {
        if (s.find("..") != std::string::npos)
            return std::string();
        return s;
    }

    std::string DeleteFileHandler::makeRelativeFromLocation(const std::string &requestTarget) const {
        if (locationPath_.empty())
            return stripOneLeadingSlash(requestTarget);

        if (requestTarget == locationPath_)
            return std::string();

        if (requestTarget.length() > locationPath_.length() &&
            requestTarget.compare(0, locationPath_.length(), locationPath_) == 0 &&
            requestTarget[locationPath_.length()] == '/')
        {
            return requestTarget.substr(locationPath_.length() + 1);
        }
        return std::string();
    }

    Response DeleteFileHandler::serveInternal(const Request &req) const {
        const std::string requestTarget = req.getRequestTarget();

        std::string relative = makeRelativeFromLocation(requestTarget);

        if (!locationPath_.empty()) {
            if (relative.empty() && requestTarget != locationPath_) {
                return ResponseBuilder().status(kStatusNotFound).build();
            }
        }

        if (relative.empty()) {
            return ResponseBuilder().status(kStatusNotFound).build();
        }

        // relative = normalizeRelative(relative); // 必要なら有効化

        std::string path = docRootConfig_.getRoot();
        if (!path.empty() && path[path.length() - 1] != '/')
            path += '/';
        path += relative;


        struct stat st;
        if (stat(path.c_str(), &st) == -1) {
            if (errno == ENOENT)
                return ResponseBuilder().status(kStatusNotFound).build();
            return ResponseBuilder().status(kStatusInternalServerError).build();
        }
        if (!S_ISREG(st.st_mode)) {
            return ResponseBuilder().status(kStatusForbidden).build();
        }
        errno = 0;
        if (std::remove(path.c_str()) != 0) {
            return ResponseBuilder().status(kStatusInternalServerError).build();
        }
        return ResponseBuilder().status(kStatusNoContent).build();
    }

} // namespace http

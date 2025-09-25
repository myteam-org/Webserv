#pragma once

#include "http/handler/handler.hpp"
#include "config/context/documentRootConfig.hpp"

namespace http {

    class DeleteFileHandler : public IHandler {
    public:
        explicit DeleteFileHandler(const DocumentRootConfig &docRootConfig);
        DeleteFileHandler(const DocumentRootConfig &docRootConfig,
                          const std::string &locationPath);
        virtual ~DeleteFileHandler();

        Either<IAction *, Response> serve(const Request &request);

    private:
        DocumentRootConfig docRootConfig_;
        std::string locationPath_; // "/upload" など。空なら無指定。

        Response serveInternal(const Request &req) const;
        static std::string stripOneLeadingSlash(const std::string &s);
        static std::string normalizeRelative(const std::string &s); // 任意 (../ 防止用)
        std::string makeRelativeFromLocation(const std::string &requestTarget) const;

        // コピー禁止（任意）
        DeleteFileHandler(const DeleteFileHandler&);
        DeleteFileHandler& operator=(const DeleteFileHandler&);
    };

} // namespace http

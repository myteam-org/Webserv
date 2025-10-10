#pragma once

#include "config/context/documentRootConfig.hpp"
#include "http/handler/handler.hpp"
#include "http/handler/file/static.hpp"
#include "http/handler/file/cgi.hpp"
#include "utils/string.hpp"

namespace http {

class FileOrCgiHandler : public IHandler {
public:
    explicit FileOrCgiHandler(const DocumentRootConfig& docRoot);

    virtual Either<IAction*, Response> serve(const Request& req);

private:
    bool isCgiTarget_(const std::string& full) const;

    DocumentRootConfig      docRoot_;
    StaticFileHandler       static_;
    CgiHandler              cgi_;
};
} // namespace http

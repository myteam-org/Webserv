#pragma once

#include "../handler.hpp"
#include "config/context/documentRootConfig.hpp"
#include "http/request/request.hpp"
#include "http/response/response.hpp"
#include "utils/types/either.hpp"
#include "utils/types/option.hpp"

namespace http {

class UploadFileHandler : public IHandler {
public:
    explicit UploadFileHandler(const DocumentRootConfig &documentRootConfig);
    
    Either<IAction *, Response> serve(const Request &request);

private:
    const DocumentRootConfig documentRootConfig_;
    
    Response serveInternal(const Request &request) const;
    Response saveUploadedFile(const Request &request) const;
    
    static std::string determineFilename(const types::Option<std::string> &contentTypeHeader);
    Response saveFileToPath(const std::vector<char> &body,
                           const std::string &uniqueFilename,
                           const std::string &uploadPath) const;
    static Response buildSuccessResponse(const std::string &filename, size_t fileSize);
    
    static std::string generateUniqueFilename(const std::string &originalName);
    static bool isValidFileType(const std::string &filename);
    static bool isValidFileSize(size_t size);
};

} // namespace http

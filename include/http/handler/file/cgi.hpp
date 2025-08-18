#pragma once

#include "handler/handler.hpp"

namespace http {
class CgiHandler : public IHandler {
   public:
    explicit CgiHandler(const DocumentRootConfig &docRootConfig);
    Either<IAction *, Response> serve(const Request &req);

   private:
    DocumentRootConfig docRootConfig_;

    // 内部実装
    inline Response serveInternal(const Request& req);
    inline bool isCgiTarget(const Request& req,
                     std::string* scriptPath,
                     std::string* pathInfo) const;
    inline bool buildCgiEnv(const Request& req,
                     const std::string& scriptName,
                     const std::string* pathInfo,
                     std::vector<std::string>* env) const;
    inline bool executeCgi(const std::vector<std::string>& argv,
                    const std::vector<std::string>& env,
                    const std::vector<char>& stdinBody,
                    std::string* stdoutBuf,
                    int* exitCode) const;
    
    Response parseCgiAndBuildResponse(const std::string& cgiOut) const;

    // utils
    static bool splitHeadersBody(const std::string& all, std::string* headers, std::string* body);
};
} // namespace http

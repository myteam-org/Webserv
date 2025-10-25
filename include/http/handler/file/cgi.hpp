#pragma once

#include "http/handler/handler.hpp"
#include "config/context/documentRootConfig.hpp"


namespace http {

class CgiHandler : public IHandler {
   public:
    explicit CgiHandler(const DocumentRootConfig &docRootConfig);
    Either<IAction *, Response> serve(const Request &req);

   private:
    DocumentRootConfig docRootConfig_;

    // 内部実装
    Response serveInternal(const Request& req) const;
    bool isCgiTarget(const Request& req,
                     std::string* scriptPath,
                     std::string* pathInfo) const;
    bool buildCgiEnv(const Request& req,
                     const std::string& scriptName,
                     const std::string* pathInfo,
                     std::vector<std::string>* env) const;
    bool executeCgi(const std::vector<std::string>& argv,
                    const std::vector<std::string>& env,
                    const std::vector<char>& stdinBody,
                    std::string* stdoutBuf,
                    int* exitCode) const;
    Either<IAction*, Response> prepareCgi(const Request& req);
    Response parseCgiAndBuildResponse(const std::string& cgiOut) const;
};

} // namespace http

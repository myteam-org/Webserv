#pragma once

#include "handler/handler.hpp"
#include "config/context/documentRootConfig.hpp"

namespace http {

    struct CgiSpawn {
        pid_t pid;
        int rfd; // 子->親 (親が読む)
        int wfd; // 親->子 (親が書く)
    };

class CgiHandler : public IHandler {
   public:
    explicit CgiHandler(const DocumentRootConfig &docRootConfig);
    Either<IAction *, Response> serve(const Request &req);
    Response parseCgiAndBuildResponse(const std::string& cgiOut) const;

   private:
    DocumentRootConfig docRootConfig_;

    // 内部実装
    // Response serveInternal(const Request& req) const;
    bool isCgiTarget(const Request& req,
                     std::string* scriptPath,
                     std::string* pathInfo) const;
    bool buildCgiEnv(const Request& req,
                     const std::string& scriptName,
                     const std::string* pathInfo,
                     std::vector<std::string>* env) const;
    bool spawnCgiProcess(const std::vector<std::string>& argv,
                         const std::vector<std::string>& env,
                         CgiSpawn* out) const;
    IAction* createCgiTask(const std::vector<std::string>& argv,
                           const std::vector<std::string>& env,
                           const std::vector<char>& stdinBody) const;

};

} // namespace http

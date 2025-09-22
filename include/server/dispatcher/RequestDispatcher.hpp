#pragma once
#include <string>
#include "server/dispatcher/DispatchResult.hpp"
#include "http/response/response.hpp"
#include "http/response/builder.hpp"
#include "server/connection/Connection.hpp"
#include "http/virtual_server.hpp"
#include "server/resolver/EndpointResolver.hpp"

class RequestDispatcher {
public:
    RequestDispatcher(EndpointResolver& resolver);
    ~RequestDispatcher();
    // ClientHandler から呼ばれる単一ステップ
    DispatchResult step(Connection& c);

    // CGI stdout/ stdin 用のモック（今は形だけ）
    DispatchResult onCgiStdout(Connection& c);
    DispatchResult onCgiStdin(Connection& c);
    DispatchResult emitError(Connection& c, http::HttpStatusCode status, const std::string& plain);

private:
    // std::map<std::string, VirtualServer*> vServers_;
    EndpointResolver& resovler_;
    DispatchResult dispatchNext(Connection& c, http::Request& req);
    // CGI
    DispatchResult startCgi(Connection& c, const std::string& scriptPath);
    // Response を直列化して送信キューへ積む（WriteBuffer の薄いアダプタ）
    void enqueueResponse(Connection& c, const http::Response& resp);
    bool shouldClose(const http::Request& req) const;
    bool isCgiTarget(const std::string& path) const;
    bool wantsCgi(Connection& c) const;
    http::Response buildErrorResponse(VirtualServer* vs, http::HttpStatusCode status, const std::string& plain);
};

#pragma once
#include <string>
#include "DispatchResult.hpp"
#include "http/response/response.hpp"
#include "http/response/builder.hpp"
#include "server/connection/Connection.hpp"
#include "http/virtual_server.hpp"

class RequestDispatcher {
public:
    RequestDispatcher() {}
    ~RequestDispatcher() {}

    // ClientHandler から呼ばれる単一ステップ
    DispatchResult step(Connection& c);

    // CGI stdout/ stdin 用のモック（今は形だけ）
    DispatchResult onCgiStdout(Connection& c);
    DispatchResult onCgiStdin(Connection& c);

    // vhost 決定（モック）
    void ensureVhost(Connection& c);

private:
    DispatchResult dispatchNext(Connection& c, http::Request& req);
    DispatchResult handleGet(Connection& c, const http::Request& req);
    // DispatchResult handleHead(Connection& c, const http::Request& req);
    DispatchResult handlePost(Connection& c, const http::Request& req);
    DispatchResult handleDelete(Connection& c, const http::Request& req);
    // CGI
    DispatchResult startCgi(Connection& c, const std::string& scriptPath);

    // Response を直列化して送信キューへ積む（WriteBuffer の薄いアダプタ）
    void enqueueResponse(Connection& c, const http::Response& resp);
    bool shouldClose(const http::Request& req) const;
    bool isCgiTarget(const std::string& path) const;
    // 便利関数（パス解決などは雑にモック）
    std::string resolveTargetPath(Connection& c) const;
    std::string resolveFilesystemPath(const http::Request& req) const;
    bool wantsCgi(Connection& c) const;
    bool isHead(Connection& c) const;
};

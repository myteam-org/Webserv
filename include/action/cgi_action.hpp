#pragma once
#include "action/action.hpp"
#include <vector>
#include "http/handler/file/cgi.hpp"

namespace http { class Response; class Request; class CgiHandler; }

struct PreparedCgi {
    std::vector<std::string> argv;
    std::vector<std::string> env;
    // 親→子 に渡す stdin ソース（Request の body をそのまま使う想定）
    const std::vector<char>* stdinBody;

    // 既存の「CGI出力→HTTPレスポンス」変換を再利用するためのコールバック
    // Response CgiHandler::parseCgiAndBuildResponse(const std::string&) const
    const http::CgiHandler* owner; 
    http::Response (http::CgiHandler::*parseFn)(const std::string&) const;

    PreparedCgi() : stdinBody(0), owner(0), parseFn(0) {}
};

class CgiActionPrepared : public IAction {
public:
    explicit CgiActionPrepared(const PreparedCgi& p);
    virtual ~CgiActionPrepared() {}
    //Dispatcher が検知して StartCgi する。
    void execute(ActionContext &ctx);
    const PreparedCgi& payload() const { return preparedCgi_; }
private:
    PreparedCgi preparedCgi_;
};


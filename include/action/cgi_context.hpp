#pragma once
#include <string>
#include <vector>
#include <ctime>
#include "action/cgi_action.hpp"

struct CgiContext {
    pid_t pid;
    int   fd_in;
    int   fd_out;
    time_t t0;

    // 受信したCGI出力（ヘッダもボディも “そのまま” ）
    std::string rawOut;

    // 入力（親→子）
    const std::vector<char>* stdinBody;
    size_t written;

    // 最終パーサ
    const http::CgiHandler* owner;
    http::Response (http::CgiHandler::*parseFn)(const std::string&) const;

    CgiContext() : pid(-1), fd_in(-1), fd_out(-1), t0(std::time(0)),
                   stdinBody(0), written(0), owner(0), parseFn(0) {}
};
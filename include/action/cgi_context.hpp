#pragma once
#include <string>
#include <vector>
#include <ctime>
#include "action/cgi_action.hpp"

// struct CgiContext {
//     pid_t pid;
//     int   fd_in;
//     int   fd_out;
//     time_t t0;

//     // 受信したCGI出力（ヘッダもボディも “そのまま” ）
//     std::string rawOut;

//     // 入力（親→子）
//     const std::vector<char>* stdinBody;
//     size_t written;

//     // 最終パーサ
//     const http::CgiHandler* owner;
//     http::Response (http::CgiHandler::*parseFn)(const std::string&) const;

//     CgiContext() : pid(-1), fd_in(-1), fd_out(-1), t0(std::time(0)),
//                    stdinBody(0), written(0), owner(0), parseFn(0) {}
// };

// cgi_context.hpp (C++98)
#pragma once
#include <vector>
#include <ctime>
#include <sys/types.h>
#include "io/input/read/buffer.hpp"
#include "io/input/write/buffer.hpp"

namespace http { class CgiHandler; class Response; }

class CgiContext {
public:
    CgiContext();
    ~CgiContext();

public:
    typedef http::Response (http::CgiHandler::*ParseFn)(const std::string&) const;
    void setProc(pid_t pid, int fd_in, int fd_out, time_t now);
    void setStdinBody(const std::vector<char>* body);
    const std::vector<char>* getStdinBody() const; 
    time_t startTime() const;
    void   terminateChild(); // SIGTERM -> SIGKILL は Server 側でやるなら呼び出しのみ

    // 付帯情報
    void setOwner(const http::CgiHandler* owner, ParseFn parse);
    http::Response invokeParser() const;
    int getFdIn() const;
    int getFdOut() const;
    void setFdIn(int fd);
    void setFdOut(int fd);
    pid_t getPid() const;
    size_t getWritten() const;
    void setWritten(size_t writeSize);
    std::string getRawOut();
    void setRawOut(std::string raw);

private:
    CgiContext(const CgiContext&);
    CgiContext& operator=(const CgiContext&);
    pid_t pid_;
    int   fd_in_;
    int   fd_out_;
    time_t t0_;

    // --- 入力: 親→子 ---
    const std::vector<char>* stdinBody_;
    size_t written_;
    // 受信したCGI出力（ヘッダもボディも “そのまま” ）
    std::string rawOut_;
    const http::CgiHandler* owner_;
    ParseFn                 parseFn_;
    // 制限
    size_t maxOutput_; // cgi_max_output_size
};

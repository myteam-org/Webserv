#pragma once

struct CgiFds {
    int stdin_fd;   // -1 でモック
    int stdout_fd;  // -1 でモック
};

struct DispatchResult {
    enum Kind {
        kNone,
        kArmOut,    // 書き込み可能にしてほしい
        kStartCgi,  // CGI の fd を epoll に登録してほしい
        kDone,      // 仕事終わり(主にCGI)
        kClose      // 接続を閉じる
    } kind;

    CgiFds cgi;
    DispatchResult(Kind k = kNone) : kind(k) { cgi.stdin_fd = -1; cgi.stdout_fd = -1; }

    static DispatchResult ArmOut()   { return DispatchResult(kArmOut); }
    static DispatchResult StartCgi(const CgiFds& fds) { DispatchResult r(kStartCgi); r.cgi = fds; return r; }
    static DispatchResult Done()     { return DispatchResult(kDone); }
    static DispatchResult Close()    { return DispatchResult(kClose); }
    bool isNone()     const { return kind == kNone; }
    bool isArmOut()   const { return kind == kArmOut; }
    bool isStartCgi() const { return kind == kStartCgi; }
    bool isDone()     const { return kind == kDone; }
    bool isClose()    const { return kind == kClose; }
};

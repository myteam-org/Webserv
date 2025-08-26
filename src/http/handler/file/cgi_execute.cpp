#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/epoll.h>

#include <string>
#include <vector>

#include "cgi.hpp"

namespace http {

namespace {

const int kStepMs = 50; // 1回の poll 待ち
const int kTotalMs = 30000; // 総タイムアウト 30s
const int EXIT_EXEC_FAILED = 127;
const int STATUS128 = 128;
const std::size_t kBufSize = 4096;
const std::size_t kReserve = 8192;

struct CgiIoCtx {
    int epfd;
    pid_t pid;
    int wfd;
    int rfd;
    const char* wPtr;
    ssize_t wRemain;
    bool wClosed;
    bool rClosed;
    int elapsed;
    int exitCode;
    std::string* out;
};

// Low-level FD / pipe utils
bool openPipes(int inPipe[2], int outPipe[2]);
void closePipe(int pipe[2]);
void safeClose(int* fd);
bool setCloexec(int fd);

// child process
void execChildAndExit(const std::vector<std::string>& argv,
                      const std::vector<std::string>& env, 
                      int inPipe[2], int outPipe[2]);
void buildVecPtr(const std::vector<std::string>& value,
                 std::vector<char*>* out);
        
// parent process
bool parentProcess(pid_t pid, int wfd, int rfd,
                   const std::vector<char>& stdinBody,
                   std::string* out, int* exitCode);
            
// utils for parent process
bool initCtx(CgiIoCtx* ctx, pid_t pid, int wfd, int rfd,
    const std::vector<char>& body, std::string* out);
bool setNonblock(int fd);
bool registerEpoll(CgiIoCtx* ctx);
// epoll を使った CGI の入出力を最後まで回し切るメインループ
bool ioLoop(CgiIoCtx* ctx);
// epoll_wait → 単発の read/write → 子の生存確認 を1回だけ
bool stepOnce(CgiIoCtx* ctx);
// helper for ioLoop
void proceedEvent(CgiIoCtx* ctx, int fd, unsigned int events);
void handleRead(CgiIoCtx* ctx);
void handleWrite(CgiIoCtx* ctx);
void closeAndSet(CgiIoCtx* ctx, int fd);
// helper for stepOnce
void checkChild(CgiIoCtx* ctx);


}  // namespace

// 本体
bool CgiHandler::executeCgi(const std::vector<std::string>& argv,
                            const std::vector<std::string>& env,
                            const std::vector<char>& stdinBody,
                            std::string* stdoutBuf, int* exitCode) const {
    if (argv.empty() || stdoutBuf == NULL || exitCode == NULL) {
        return false;
    }
    int inPipe[2] = {-1, -1};
    int outPipe[2] = {-1, -1};
    if (!openPipes(inPipe, outPipe)) {
        return false;
    }
    const pid_t pid = fork();
    if (pid < 0) {
        closePipe(inPipe);
        closePipe(outPipe);
        return false;
    }
    if (pid == 0) {
        execChildAndExit(argv, env, inPipe, outPipe);
    }
    close(inPipe[0]);
    close(outPipe[1]);
    const bool ok = parentProcess(pid, inPipe[1], outPipe[0], stdinBody, stdoutBuf,
                            exitCode);
    int status = 0;
    if (!ok) {
        kill(pid, SIGKILL);
        (void)waitpid(pid, &status, 0);
    }
    return ok;
}

namespace {

// utils for executeCgi
bool openPipes(int inPipe[2], int outPipe[2]) {
    if (pipe(inPipe) != 0) {
        return false;
    }
    if (pipe(outPipe) != 0) {
        closePipe(inPipe);
        return false;
    }
    if (!setCloexec(inPipe[0]) || !setCloexec(inPipe[1]) ||
        !setCloexec(outPipe[0]) || !setCloexec(outPipe[1])) {
        closePipe(inPipe);
        closePipe(outPipe);
        return false;
    }
    return true;
}

void closePipe(int pipe[2]) {
    if (pipe[0] >= 0) {
        close(pipe[0]);
        pipe[0] = -1;
    }
    if (pipe[1] >= 0) {
        close(pipe[1]);
        pipe[1] = -1;
    }
}

void safeClose(int* fd) {
    if (fd && *fd >= 0) {
        close(*fd);
        *fd = -1;
    }
}

bool setCloexec(int fd) {
    const int flags = fcntl(fd, F_GETFD);
    if (flags < 0) {
        return false;
    }
    // exec したときにこのFDを自動で閉じる
    return fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == 0;
}

// child process
void execChildAndExit(const std::vector<std::string>& argv,
                      const std::vector<std::string>& env, int inPipe[2],
                      int outPipe[2]) {
    if (dup2(inPipe[0], STDIN_FILENO) < 0) {
        _exit(EXIT_EXEC_FAILED);
    }
    if (dup2(outPipe[1], STDOUT_FILENO) < 0) {
        _exit(EXIT_EXEC_FAILED);
    }
    closePipe(inPipe);
    closePipe(outPipe);

    std::vector<char*> argvp;
    buildVecPtr(argv, &argvp);
    std::vector<char*> envp;
    buildVecPtr(env, &envp);

    execve(argv[0].c_str(), &argvp[0], &envp[0]);
    _exit(EXIT_EXEC_FAILED);
}

// utils for execChildAndExit
// std::vector<std::string> を execve 用の char* const argv[] 形式に変換する
void buildVecPtr(const std::vector<std::string>& value,
                 std::vector<char*>* out) {
    if (!out) {
        return;
    }
    out->clear();
    out->reserve(value.size() + 1);
    for (std::size_t i = 0; i < value.size(); ++i) {
        out->push_back(const_cast<char*>(value[i].c_str()));
    }
    out->push_back(0);
}

// parent process
bool parentProcess(pid_t pid, int wfd, int rfd, const std::vector<char>& body,
                   std::string* out, int* exitCode) {
    if (out == NULL || exitCode == NULL) {
        return false;
    }
    CgiIoCtx ctx;
    if (!initCtx(&ctx, pid, wfd, rfd, body, out)) {
        safeClose(&wfd);
        safeClose(&rfd);
        int status = 0;
        (void)waitpid(pid, &status, 0);
        return false;
    }
    bool ok = ioLoop(&ctx);
    *exitCode = ctx.exitCode;
    safeClose(&ctx.epfd);
    return ok;
}

// utils for parent process
bool initCtx(CgiIoCtx* ctx, pid_t pid, int wfd, int rfd,
            const std::vector<char>& body, std::string* out) {
    ctx->epfd = epoll_create1(EPOLL_CLOEXEC);
    if (ctx->epfd < 0) {
        return false;
    }
    ctx->pid = pid;
    ctx->wfd = wfd;
    ctx->rfd = rfd;
    ctx->wPtr = body.empty() ? NULL : &body[0];
    ctx->wRemain = static_cast<ssize_t>(body.size());
    ctx->wClosed = (ctx->wRemain == 0);
    ctx->rClosed = false;
    ctx->elapsed = 0;
    ctx->exitCode = 0;
    ctx->out = out;
    out->clear();
    out->reserve(kReserve);
    (void)setNonblock(ctx->rfd);
    (void)setNonblock(ctx->wfd);
    if (!registerEpoll(ctx)) {
        safeClose(&ctx->epfd);
        return false;
    }
    return true;
}

// helper for initCtx
bool setNonblock(int fd) {
    if (fd < 0) {
        return false;
    }
    const int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        return false;
    }
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK) == 0;
}

// helper for initCtx
bool registerEpoll(CgiIoCtx* ctx) {
    bool wOk = true;
    bool rOk = true;
    struct epoll_event event;

    // epollに監視を登録「書けるようになったら通知して」
    if (!ctx->wClosed && ctx->wfd >= 0) {
        memset(&event, 0, sizeof(event));
        event.events = EPOLLOUT;
        event.data.fd = ctx->wfd;
        wOk = (epoll_ctl(ctx->epfd, EPOLL_CTL_ADD, ctx->wfd, &event) == 0); 
    }
    // epollに監視を登録「読めるようになったら通知して」
    if (!ctx->rClosed && ctx->rfd >= 0) {
        memset(&event, 0, sizeof(event));
        event.events = EPOLLIN;
        event.data.fd = ctx->rfd;
        rOk = (epoll_ctl(ctx->epfd, EPOLL_CTL_ADD, ctx->rfd, &event) == 0); 
    }
    if (ctx->wClosed) {
        safeClose(&ctx->wfd);
    }
    return wOk && rOk;
}

// epoll 駆動で CGI 子プロセスとの I/O を完了/失敗まで進めるメインループ。
//   - stepOnce() を繰り返し呼び、rfd を読み切り & wfd を書き切るまで継続
//   - 途中で stepOnce() が false を返したらタイムアウト/異常として false を返す
//   - 正常終了時は waitpid() で子を回収し、ctx->exitCode を確定
// Returns:
//   true  = 正常終了（I/O 完了→wait まで完了）
//   false = タイムアウト等で中断（kill は呼び出し側で実施）
// Side effects:
//   ctx->out に出力を蓄積, ctx->rClosed/wClosed/exitCode を更新
bool ioLoop(CgiIoCtx* ctx) {
    while (!ctx->rClosed || !ctx->wClosed) {
        // epoll 駆動の “I/Oポンプ” の1回分
        if (!stepOnce(ctx)) {
            return false;
        }
    }
    int status = 0;
    (void)waitpid(ctx->pid, &status, 0);
    if (ctx->exitCode == 0) {
        if (WIFEXITED(status)) {
            ctx->exitCode = WEXITSTATUS(status);
            return true;
        }
        if (WIFSIGNALED(status)) {
            ctx->exitCode = STATUS128 + WTERMSIG(status);
            return true;
        }
        ctx->exitCode = -1;
    }
    return true;
}

// CGI 子プロセスとのノンブロッキング入出力を “1ステップ” だけ進めるための関数
bool stepOnce(CgiIoCtx* ctx) {
    // rfd と wfd の“準備完了イベント”を受け取るためのバッファ
    epoll_event evs[2];
    const int readyEvents = epoll_wait(ctx->epfd, evs, 2, kStepMs);

    if (readyEvents < 0) {
        return true;
    }
    if (readyEvents == 0) {
        ctx->elapsed += kStepMs;
        // アイドル（無イベント）状態の総時間で CGI をタイムアウトさせる
        if (ctx->elapsed >= kTotalMs) {
            safeClose(&ctx->rfd);
            safeClose(&ctx->wfd);
            return false;
        }
        return true; // イベントなしはここで1ステップ終了
    }
    ctx->elapsed = 0; // イベントが来たらリセット
    for (int i = 0; i < readyEvents; ++i) {
        // イベントが起きたFD
        const int fd = evs[i].data.fd;
        // 何の種類のイベントが起きたかのビットマスク
        const unsigned int events = evs[i].events; 
        proceedEvent(ctx, fd, events);
    }
    checkChild(ctx);
    return true;
}

// helper function for stepOnce
void proceedEvent(CgiIoCtx* ctx, int fd, unsigned int events) {
    if (fd == ctx->rfd && (events & EPOLLIN)) { // 読み
        handleRead(ctx);
    }
    if (fd == ctx->wfd && (events & EPOLLOUT)) { // 書き
        handleWrite(ctx);
    }
    // rfd: EPOLLINが無いときだけHUP/ERRで閉じる（読み残し保護）
    if (fd == ctx->rfd && !(events & EPOLLIN) && 
        (events & (EPOLLERR | EPOLLHUP))) {
        closeAndSet(ctx, fd);
    }
    // wfd: HUP/ERRなら即クローズ
    if (fd == ctx->wfd && (events & (EPOLLHUP | EPOLLERR))) {
        closeAndSet(ctx, fd);
    }
}

// 読む
void handleRead(CgiIoCtx* ctx) {
    char buf[kBufSize];
    const ssize_t byte = ::read(ctx->rfd, buf, sizeof(buf));
    if (byte > 0) {
        ctx->out->append(buf, static_cast<std::size_t>(byte));
        return;
    }
    if (byte == 0) { // EOF
        safeClose(&ctx->rfd);
        ctx->rClosed = true;
    }
    // n < 0 は何もしない（errno は見ない）
}

// 書く
void handleWrite(CgiIoCtx* ctx) {
    if (ctx->wRemain <= 0) {
        if (!ctx->wClosed) {
            safeClose(&ctx->wfd);
            ctx->wClosed = true;
        }
        return;
    }
    const ssize_t byte = ::write(ctx->wfd, ctx->wPtr, static_cast<std::size_t>(ctx->wRemain));
    if (byte > 0) {
        ctx->wPtr += byte;
        ctx->wRemain -= byte;
        if (ctx->wRemain <= 0) {
            safeClose(&ctx->wfd);
            ctx->wClosed = true;
        }
    }
    // byte <= 0 は何もしない（errno は見ない）
}

// helper for proceedEvent
void closeAndSet(CgiIoCtx* ctx, int fd) {
    if (fd == ctx->rfd) {
        safeClose(&ctx->rfd);
        ctx->rClosed = true;
    }
    if (fd == ctx->wfd) {
        safeClose(&ctx->wfd);
        ctx->wClosed = true;
    } 
}

// helper for stepOnce
void checkChild(CgiIoCtx* ctx) {
    int status = 0;
    const pid_t got = waitpid(ctx->pid, &status, WNOHANG);
    if (got != ctx->pid) {
        return;
    }
    if (WIFEXITED(status)) {
        ctx->exitCode = WEXITSTATUS(status);
        return;
    }
    if (WIFSIGNALED(status)) {
        ctx->exitCode = STATUS128 + WTERMSIG(status);
        return;
    }
    ctx->exitCode = -1;
}

}  // namespace

}  // namespace http

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
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

void safeClose(int* fd) {
    if (fd && *fd >= 0) {
        close(*fd);
        *fd = -1;
    }
}

int makeNonblock(int fd) {
    int fdFlags = fcntl(fd, F_GETFL, 0);
    if (fdFlags < 0) {
        return -1;
    }
    return fcntl(fd, F_SETFL, fdFlags | O_NONBLOCK);
}

bool epAdd(int epfd, int fd, u_int32_t evs) {
    epoll_event event;
    event.events = evs;
    event.data.fd = fd;
    return epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event) == 0;
}

void iniCtx(CgiIoCtx* ctx, pid_t pid, int wfd, int rfd,
            const std::vector<char>& body, std::string* out) {
    ctx->epfd = epoll_create1(0);
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
    if (ctx->rfd >= 0) { // nonblockingをセット
        makeNonblock(ctx->rfd);
    }
    if (ctx->wfd >= 0) { // nonblockingをセット
        makeNonblock(ctx->wfd);
    }
    if (!ctx->wClosed && ctx->wfd >= 0) { // epollに監視を登録「書けるようになったら通知して」
        epAdd(ctx->epfd, ctx->wfd, EPOLLOUT);
    }
    if (!ctx->rClosed && ctx->rfd >= 0) { // epollに監視を登録「読めるようになったら通知して」
        epAdd(ctx->epfd, ctx->rfd, EPOLLIN);
    }
    if (ctx->wClosed) {
        safeClose(&wfd);
    }
}

void handleRead(CgiIoCtx* ctx) {
    char buf[kBufSize];
    for(;;) {
        ssize_t byte = ::read(ctx->rfd, buf, sizeof(buf));
        if (byte > 0) {
            ctx->out->append(buf, static_cast<std::size_t>(byte));
            continue;
        }
        if (byte == 0) {
            safeClose(&ctx->rfd);
            ctx->rClosed = true;
            break;
        }
    }
}

void handleWrite(CgiIoCtx* ctx) {
    while (ctx->wRemain > 0) {
        ssize_t byte = ::write(ctx->wfd, ctx->wPtr, static_cast<std::size_t>(ctx->wRemain));
        if (byte > 0) {
            ctx->wPtr += byte;
            ctx->wRemain -= byte;
            continue;
        }
        break;
    }
    if (ctx->wRemain <= 0) {
        safeClose(&ctx->wfd);
        ctx->wClosed = true;
    }
}

void checkChild(CgiIoCtx* ctx) {
    int status = 0;
    pid_t got = waitpid(ctx->pid, &status, WNOHANG);
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




// utils for executeCgi()
bool openPipes(int inPipe[2], int outPipe[2]);
bool setCloexec(int fd);
void closePipe(int pipe[2]);

// child process
void execChildAndExit(const std::vector<std::string>& argv,
                      const std::vector<std::string>& env, int inPipe[2],
                      int outPipe[2]);
// utils fo execChildAndExit()
void buildVecPtr(const std::vector<std::string>& value,
    std::vector<char*>* out);

// parent process
bool parentProcess(pid_t pid, int wfd, int rfd,
                   const std::vector<char>& stdinBody, std::string* out,
                   int* exitCode);
// utils for parent process
bool doWrite(pid_t pid, int wfd, int rfd, const char* ptr, ssize_t remain);
bool doRead(pid_t pid, int rfd, std::string* out);


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

bool setCloexec(int fd) {
    const int flags = fcntl(fd, F_GETFD);
    if (flags < 0) {
        return false;
    }
    // exec したときにこのFDを自動で閉じる
    return fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == 0;
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
    if (out == NULL || exitCode == NULL) return false;
    out->clear();
    int elapsed = 0;
    const char* ptr = body.empty() ? NULL : &body[0];
    const ssize_t remain = static_cast<ssize_t>(body.size());

    if (!doWrite(pid, wfd, rfd, ptr, remain)) return false;
    if (!doRead(pid, rfd, out)) return false;
    int status = 0;
    if (waitpid(pid, &status, 0) < 0) {
        // sigaction(SIGPIPE, &sa_old, NULL);
        return false;
    }
    if (WIFEXITED(status)) {
        *exitCode = WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
        *exitCode = STATUS128 + WTERMSIG(status);
    } else {
        *exitCode = -1;
    }
    // sigaction(SIGPIPE, &sa_old, NULL);
    return true;
}

// utils for parent process
bool doWrite(pid_t pid, int wfd, int rfd, const char* ptr, ssize_t remain) {
    while (remain > 0) {
        const ssize_t byte = write(wfd, ptr, static_cast<size_t>(remain));
        if (byte < 0) {
            if (errno == EINTR) {
                continue;
            }
            close(wfd);
            close(rfd);
            (void)waitpid(pid, 0, 0);
            return false;
        }
        remain -= byte;
        ptr += byte;
    }
    close(wfd);
    return true;
}

bool doRead(pid_t pid, int rfd, std::string* out) {
    out->reserve(kReserve);
    while (true) {
        char buf[kBufSize];
        const ssize_t byte = read(rfd, buf, sizeof(buf));
        if (byte < 0) {
            if (errno == EINTR) {
                continue;
            }
            close(rfd);
            (void)waitpid(pid, 0, 0);
            return false;
        }
        if (byte == 0) {
            break;
        }
        out->append(buf, static_cast<size_t>(byte));
    }
    close(rfd);
    return true;
}

}  // namespace

}  // namespace http

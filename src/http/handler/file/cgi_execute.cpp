#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

#include <string>
#include <vector>

#include "cgi.hpp"

namespace http {

namespace {

const int EXIT_EXEC_FAILED = 127;
const int STATUS128 = 128;
const std::size_t kBufSize = 4 * 1024;
const std::size_t kReserve = 8 * 1024;

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
bool parentProcess(pid_t pid, int writeFd, int readFd,
                   const std::vector<char>& stdinBody, std::string* stdoutBuf,
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
    if (argv.empty() || stdoutBuf == 0 || exitCode == 0) {
        return false;
    }
    int inPipe[2] = {-1, -1};
    int outPipe[2] = {-1, -1};
    if (!openPipes(inPipe, outPipe)) {
        return false;
    }
    pid_t pid = fork();
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
    bool ok = parentProcess(pid, inPipe[1], outPipe[0], stdinBody, stdoutBuf,
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
    int flags = fcntl(fd, F_GETFD);
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
    if (out == 0 || exitCode == 0) return false;

    const char* ptr = body.empty() ? 0 : &body[0];

    struct sigaction sa_old{};
    struct sigaction sa_new{};
    sa_new.sa_handler = SIG_IGN;
    sigemptyset(&sa_new.sa_mask);
    sa_new.sa_flags = 0;
    sigaction(SIGPIPE, &sa_new, &sa_old);

    ssize_t remain = static_cast<ssize_t>(body.size());
    if (!doWrite(pid, wfd, rfd, ptr, remain)) return false;
    if (!doRead(pid, rfd, out)) return false;
    int status = 0;
    if (waitpid(pid, &status, 0) < 0) {
        sigaction(SIGPIPE, &sa_old, NULL);
        return false;
    }
    if (WIFEXITED(status)) {
        *exitCode = WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
        *exitCode = STATUS128 + WTERMSIG(status);
    } else {
        *exitCode = -1;
    }
    sigaction(SIGPIPE, &sa_old, NULL);
    return true;
}

// utils for parent process
bool doWrite(pid_t pid, int wfd, int rfd, const char* ptr, ssize_t remain) {
    while (remain > 0) {
        ssize_t byte = write(wfd, ptr, static_cast<size_t>(remain));
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
        ssize_t byte = read(rfd, buf, sizeof(buf));
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

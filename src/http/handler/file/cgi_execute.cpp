#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
// #include <sys/epoll.h>

#include <string>
#include <vector>

#include "cgi.hpp"
#include "action.hpp"
#include "cgiTask.hpp"

namespace http {

namespace {

const int EXIT_EXEC_FAILED = 127;

// Low-level FD / pipe utils
bool openPipes(int inPipe[2], int outPipe[2]);
void closePipe(int pipe[2]);
void safeClose(int* fd);
bool setCloexec(int fd);
bool setNonblock(int fd);

// child process
void execChildAndExit(const std::vector<std::string>& argv,
                      const std::vector<std::string>& env, 
                      int inPipe[2], int outPipe[2]);
void buildVecPtr(const std::vector<std::string>& value,
                 std::vector<char*>* out);

}  // namespace

// 本体
bool CgiHandler::spawnCgiProcess(const std::vector<std::string>& argv,
                                 const std::vector<std::string>& env,
                                 CgiSpawn* out) const {
    if (!out) {
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
    (void)setNonblock(inPipe[1]);
    (void)setNonblock(outPipe[0]);

    out->pid = pid;
    out->wfd = inPipe[1];
    out->rfd = outPipe[0];
    return true;
}

IAction* CgiHandler::createCgiTask(const std::vector<std::string>& argv,
                                   const std::vector<std::string>& env,
                                   const std::vector<char>& stdinBody) const {
    CgiSpawn spawn;

    if (!spawnCgiProcess(argv, env, &spawn)) {
        return NULL;
    }
    return new CgiTask(spawn.pid, spawn.rfd, spawn.wfd, stdinBody, this);
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

bool setNonblock(int fd) {
    if (fd < 0) {
        return false;
    }
    const int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        return false;
    }
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK) == 0;
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

} // namespace
} // namespace http

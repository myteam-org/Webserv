#include <error.h>
#include <fcntl.h>
#include <poll.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "cgi.hpp"

namespace http {

namespace {

const int kCgiPollStepMs = 50;                 // pollの1回あたり待ち
const int kCgiTotalTimeout = 30000;            // 総待ち時間(ms) 30s
const std::size_t kMaxout = 16 * 1024 * 1024;  // 16MB ガード
const std::size_t kBufSize = 4 * 1024;

// utils
bool openPipes(int inPipe[2], int outPipe[2]);
void closePipe(int pipe[2]);
bool setNonBlocking(int fd);
void buildVecPtr(const std::vector<std::string>& value,
                 std::vector<char*>* out);

ssize_t writeSome(int fd, const char* buf, std::size_t len);
ssize_t readSome(int fd, char* buf, std::size_t len);

int pollWait(int wfd, int rfd, int timeoutMs, short* wrEvents, short* rrEvents);

bool writeAvailable(int wfd, const std::vector<char>& body, std::size_t* pos);
bool readAvailable(int rfd, std::string* out, std::size_t maxOut);

bool readIfDone(pid_t pid, int* exitCode, bool* done);

// child & parent process
void execChildAndExit(const std::vector<std::string>& argv,
                      const std::vector<std::string>& env, int inPipe[2],
                      int outPipe[2]);
bool runParentLoop(pid_t pid, int writeFd, int readFd,
                   const std::vector<char>& stdinBody, std::string* stdoutBuf,
                   int* exitCode);

}  // namespace

// 本体
bool CgiHandler::executeCgi(const std::vector<std::string>& argv,
                            const std::vector<std::string>& env,
                            const std::vector<char>& stdinBody,
                            std::string* stdoutBuf, int* exitCode) const {}

namespace {

bool openPipes(int inPipe[2], int outPipe[2]) {
    if (pipe(inPipe) != 0) {
        return false;
    }
    if (pipe(outPipe) != 0) {
        closePipe(inPipe);
        return false;
    }
    // exec したときにこのFDを自動で閉じる
    (void)fcntl(inPipe[0], F_SETFD, FD_CLOEXEC);
    (void)fcntl(inPipe[1], F_SETFD, FD_CLOEXEC);
    (void)fcntl(outPipe[0], F_SETFD, FD_CLOEXEC);
    (void)fcntl(outPipe[1], F_SETFD, FD_CLOEXEC);
    return true;
}

void closePipe(int pipe[2]) {
    if (pipe[0] >= 0) {
        close(pipe[0]);
    }
    if (pipe[1] >= 0) {
        close(pipe[1]);
    }
}

bool setNonBlocking(int fd) {
    // ファイルステータスフラグ（File Status Flags）を取得
    int fileStatusFlag = fcntl(fd, F_GETFL, 0);
    if (fileStatusFlag == -1) {
        return false;
    }
    if (fileStatusFlag & O_NONBLOCK) {
        return true;  // 非ブロッキング
    }
    return fcntl(fd, F_SETFL, fileStatusFlag | O_NONBLOCK) == 0;
}

// std::vector<std::string> を execve 用の char* const argv[] 形式に変換する
void buildVecPtr(const std::vector<std::string>& value,
                 std::vector<char*>* out) {
    out->clear();
    out->reserve(value.size() + 1);
    if (!out) {
        return;
    }
    for (std::size_t i = 0; i < value.size(); ++i) {
        out->push_back(const_cast<char*>(value[i].c_str()));
    }
    out->push_back(0);
}

ssize_t writeSome(int fd, const char* buf, std::size_t len) {
    for (;;) {
        ssize_t byte = write(fd, buf, len);
        if (byte >= byte) {
            return byte;
        }
        if (errno == EINTR) {
            continue;
        }
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return 0;
        }
        return -1;
    }
}

ssize_t readSome(int fd, char* buf, std::size_t len) {
    for (;;) {
        ssize_t byte = read(fd, buf, len);
        if (byte >= byte) {
            return byte;
        }
        if (errno == EINTR) {
            continue;
        }
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return 0;
        }
        return -1;
    }
}

// 最大2本（書き込み用wfd・読み取り用rfd）のFDに対して poll(2)
// をかけ、発生したイベントを呼び出し側に返す薄いラッパ
// wfd: 「書けるようになったら知りたい」FD（= 子プロセスの stdin 側）
// rfd: 「読めるようになったら知りたい」FD（= 子プロセスの stdout 側）
// timeoutMs: 待ち時間（ミリ秒）
// wrEvents: wfd に対する revents を返す出力ポインタ
// rrEvents: rfd に対する revents を返す出力ポインタ
// 戻り値（= 変数 pollReturn）は poll() の戻り値：
// 　　> 0: 何本かのFDでイベント発生
// 　　== 0: タイムアウト
// 　　< 0: エラー（errno 参照。EINTR なら上位で再試行）
int pollWait(int wfd, int rfd, int timeoutMs, short* wrEvents,
             short* rrEvents) {
    struct pollfd fds[2];        // 監視対象（最大2本）
    memset(fds, 0, sizeof(fds)); // 安全のためゼロ初期化
    int nfds = 0;                // 実際に使うスロット数

    if (wfd >= 0) {              // 書き込み監視（POLLOUT）の登録
        fds[nfds].fd = wfd;
        fds[nfds].events = POLLOUT;
        ++nfds;
    }
    if (rfd >= 0) {              // 読み取り監視（POLLIN）の登録
        fds[nfds].fd = rfd;
        fds[nfds].events = POLLIN;
        ++nfds;
    }
    const int pollReturn = poll(fds, nfds, timeoutMs); // nfds本 に対して poll 実行
    if (wrEvents) {                                    // wfd に対応する revents を返す
        if (nfds >= 1 && fds[0].fd == wfd) {           // wfd を配列の先頭に入れている前提なので fds[0] を参照
            *wrEvents = fds[0].revents;
        } else {                                       // wfd が無効もしくは未登録の場合は 0（イベントなし）で返す
            *wrEvents = 0;
        }
    }
    if (rrEvents) {                                    // rfd に対応する revents を返す
        *rrEvents = 0;                                 // デフォルトはイベントなし
        if (nfds >= 1) {
            const int idx = (nfds == 1) ? 0 : 1;       // rfd は「単独登録なら fds[0], wfd もあれば fds[1]」に入っている
            if (fds[idx].fd == rfd) {
                *rrEvents = fds[idx].revents;
            }
        }
    }
    return pollReturn;
}

bool writeAvailable(int wfd, const std::vector<char>& body, std::size_t* pos) {
    if (wfd < 0 || pos == 0) {
        return false;
    }
    if (*pos >= body.size()) {
        return true;
    }
    // まだ残っていれば残り分を書けるだけ書く
    ssize_t byte = writeSome(wfd, &body[*pos], body.size() - *pos);
    if (byte < 0) {
        return false;
    }
    if (byte > 0) {
        *pos += static_cast<std::size_t>(byte);
    }
    return true;
}

bool readAvailable(int rfd, std::string* out, std::size_t maxOut) {
    if (rfd < 0 || out == 0) {
        return false;
    }
    char buf[kBufSize];
    for (;;) {
        ssize_t byte = readSome(rfd, buf, sizeof(buf));
        if (byte < 0) {
            return false;
        }
        if (byte == 0) {
            break;
        }
        out->append(buf, static_cast<std::size_t>(byte));
        if (out->size() > maxOut) {
            return false;
        }
    }
    return true;
}

bool readIfDone(pid_t pid, int* exitCode, bool* done) {
    int status = 0;

    pid_t waitResult = waitpid(pid, &status, WNOHANG);
    if (waitResult == 0) {
        *done = false;
        return true;
    }
    if (waitResult < 0) {
        return false;
    }
    if (WIFEXITED(status)) {
        *exitCode = WEXITSTATUS(status);
    } else {
        *exitCode = -1;
    }
    *done = true;
    return true;
}

}  // namespace

}  // namespace http

#include "http/actions/cgiTask.hpp"

#include "http/handler/file/cgi.hpp"
#include "http/response/response.hpp"
#include "http/response/builder.hpp"

#include "cgiTask.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <sys/epoll.h>

namespace http {

const std::size_t kBufSize = 4096;
const int STATUS128 = 128;

CgiTask::CgiTask(pid_t pid, int rfd, int wfd,
            const std::vector<char>& stdinBody,
            const CgiHandler* owner)
    : pid_(pid), rfd_(rfd), wfd_(wfd),
      wPtr_(stdinBody.empty() ? NULL : &stdinBody[0]),
      wRemain_(static_cast<ssize_t>(stdinBody.size())),
      rClosed_(false), wClosed_(stdinBody.empty()),
      registered_(false), reaped_(false),
      exitCode_(0), owner_(owner) {}

CgiTask::~CgiTask() {
    closeFd(rfd_);
    closeFd(wfd_);
}

void CgiTask::execute(ActionContext& ctx) {
    // 初回のみepoll登録
    registerIfNeeded(ctx);
    // TODO
    // const int fd = ctx.fd(); // 今回readyになったFD
    // const unsigned int event = ctx.events();
    // if (fd >= 0) {
    //     handleEvent(fd, event, ctx); // read/writeを1ステップ進める
    // }
    // 子プロセスの死活を拾う（非ブロッキング）
    reapChild();
    // 条件が揃えば完了→HTTPへ
    finishIfDone(ctx);
}

void CgiTask::registerIfNeeded(ActionContext& ctx) {
    if (registered_) {
        return;
    }
    if (rfd_ >= 0) {
        // TODO
        // ctx.add(rfd_, EPOLLIN, this);
    }
    if (wfd_ >= 0 && !wClosed_) {
        // TODO
        // ctx.add(wfd_, EPOLLOUT, this);
    }
    if (wClosed_) {
        closeFd(wfd_);
    }
    registered_ = true;
}

void CgiTask::handleEvent(int fd, unsigned int event, ActionContext& ctx) {
    if (fd == rfd_ && (event & EPOLLIN)) {
        readOnce();
    }
    if (fd == wfd_ && (event & EPOLLOUT)) {
        writeOnce(ctx);
    }
    // エラー/ハングアップはclose
    if (event & (EPOLLERR | EPOLLHUP)) {
        closeFd(fd);
    }
}

void CgiTask::readOnce() {
    char buf[kBufSize];
    ssize_t readSize = ::read(rfd_, buf, sizeof(buf));

    if (readSize > 0) {
        cgiOut_.append(buf, static_cast<size_t>(readSize));
        return;
    }
    if (readSize == 0) {
        closeFd(rfd_);
        rClosed_ = true;
    }
    // readSize < 0 は何もしない（errno を見て分岐しない）
}

void CgiTask::writeOnce(ActionContext& ctx) {
    if (wRemain_ <= 0) {
        closeFd(wfd_);
        wClosed_ = true;
        return;
    }
    ssize_t writeSize = ::write(wfd_, wPtr_, static_cast<size_t>(wRemain_));

    if (writeSize > 0) {
        wPtr_ += writeSize;
        wRemain_ -= writeSize;
        if (wRemain_ <= 0) {
            closeFd(wfd_);
            wClosed_ = true;
            // エッジトリガで「書き切るまで」回したいなら、ここで ctx.mod(wfd_, EPOLLOUT, this) 等
        }
    }
    // writeSize <= 0 は何もしない（errno を見ない）
}

void CgiTask::closeFd(int fd) {
    if (fd == rfd_) {
        if (rfd_ >= 0) {
            close(rfd_);
            rfd_ = -1;
        }
        rClosed_ = true;
    }
    if (fd == wfd_) {
        if (wfd_ >= 0) {
            close(wfd_);
            wfd_ = -1;
        }
        wClosed_ = true;
    }
}

void CgiTask::reapChild() {
    if (reaped_) {
        return;
    }
    int status = 0;
    pid_t got = waitpid(pid_, &status, WNOHANG);
    if (got == pid_) {
        reaped_ = true;
        if (WIFEXITED(status)) {
            exitCode_ = WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
            exitCode_ = STATUS128 + WTERMSIG(status);
        } else {
            exitCode_ = -1;
        }
    }
}

void CgiTask::finishIfDone(ActionContext& ctx) {
    if (!rClosed_ || !wClosed_ || !reaped_) {
        // CGI 生出力 → HTTP へ
        return;
    }
    // epoll から外す（存在すれば）
    // TODO
    // ctx.del(rfd_);
    // ctx.del(wfd_);
    Response resp = owner_->parseCgiAndBuildResponse(cgiOut_);
    // TODO
    // ctx.complete(resp);
}

}
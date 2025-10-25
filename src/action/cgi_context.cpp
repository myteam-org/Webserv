// cgi_context.cpp
#include "action/cgi_context.hpp"

#include <signal.h>
#include <unistd.h>
#include <limits.h>   // SIZE_MAX (C++98 でも利用可)
#include <ctime>
#include "http/response/builder.hpp"

CgiContext::CgiContext()
    : pid_(-1)
    , fd_in_(-1)
    , fd_out_(-1)
    , t0_(std::time(0))
    , stdinBody_(0)
    , written_(0)
    , rawOut_()
    , owner_(0)
    , parseFn_(0)
    , maxOutput_(0) {}

CgiContext::~CgiContext() {}

void CgiContext::setProc(pid_t pid, int fd_in, int fd_out, time_t now) {
    pid_   = pid;
    fd_in_ = fd_in;
    fd_out_= fd_out;
    t0_    = now;
}

void CgiContext::setStdinBody(const std::vector<char>* body) {
    stdinBody_ = body;
    written_   = 0;
}

const std::vector<char>* CgiContext::getStdinBody() const{
    return stdinBody_;
}

time_t CgiContext::startTime() const {
    return t0_;
}

void CgiContext::terminateChild() {
    // Escalation（SIGTERM->SIGKILL）は Server 側ポリシーで実施する前提。
    // ここは「TERM を一発送る」軽いフックとして実装。
    if (pid_ > 0) {
        ::kill(pid_, SIGTERM);
    }
}

void CgiContext::setOwner(const http::CgiHandler* owner, ParseFn parse) {
    owner_   = owner;
    parseFn_ = parse;
}

int CgiContext::getFdIn() const {
    return fd_in_;
}

int CgiContext::getFdOut() const {
    return fd_out_;
}

void CgiContext::setFdIn(int fd) {
    fd_in_ = fd;
}

void CgiContext::setFdOut(int fd) {
    fd_out_ = fd;
}

pid_t CgiContext::getPid() const {
    return pid_;
}

size_t CgiContext::getWritten() const {
    return written_;
}

void CgiContext::setWritten(size_t writeSize) {
    written_ = writeSize;
}

http::Response CgiContext::invokeParser() const {
    if (!owner_ || !parseFn_) {
        LOG_ERROR("CgiContext::invokeParser : owner_ or parseFn_ is null");
        http::ResponseBuilder rb;
        rb.status(http::kStatusInternalServerError)
          .header("Content-Type", "text/plain; charset=UTF-8")
          .body("", http::kStatusInternalServerError);
        return (rb.build());
    }
    return (owner_->*parseFn_)(rawOut_);
}

std::string CgiContext::getRawOut() {
    return rawOut_;
};

void CgiContext::setRawOut(std::string raw) {
    rawOut_ = raw;
};
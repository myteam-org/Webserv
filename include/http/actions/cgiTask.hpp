#pragma once

#include "action/action.hpp"
#include <string>
#include <vector>

#include <sys/types.h>
#include <unistd.h>

namespace http {
class CgiHandler;

class CgiTask : public IAction {
   public:
    CgiTask(pid_t pid, int rfd, int wfd,
            const std::vector<char>& stdinBody,
            const CgiHandler* owner);
    virtual ~CgiTask();
    virtual void execute(ActionContext& ctx);

   private:
    void registerIfNeeded(ActionContext& ctx);
    void handleEvent(int fd, unsigned int event, ActionContext& ctx);
    void readOnce();
    void writeOnce(ActionContext& ctx);
    void closeFd(int fd);
    void reapChild();
    void finishIfDone(ActionContext& ctx);

   private:
    pid_t pid_;
    int rfd_;
    int wfd_;
    const char* wPtr_;
    ssize_t wRemain_;
    bool rClosed_;
    bool wClosed_;
    bool registered_;
    bool reaped_;
    int exitCode_;
    std::string cgiOut_;
    const CgiHandler* owner_;
};
} // namespace http

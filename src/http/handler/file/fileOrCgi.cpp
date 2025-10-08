#warning "fileOrCgi.cpp is compiled"
#include "http/handler/file/fileOrCgi.hpp"

#include "http/request/request.hpp"
#include "utils/logger.hpp"
#include <unistd.h>

namespace http {

FileOrCgiHandler::FileOrCgiHandler(const DocumentRootConfig& docRoot)
         : docRoot_(docRoot), static_(docRoot), cgi_(docRoot) {
            LOG_INFO("FileOrCgiHandler ctor: root=" + docRoot_.getRoot());
         }

Either<IAction*, Response> FileOrCgiHandler::serve(const Request& req) {
    const std::string& target = req.getRequestTarget();     // 例: "/happy.py"
    const std::string full    = docRoot_.getRoot() + target; // 例: "./www/cgi-bin/happy.py"
    LOG_INFO("FileOrCgiHandler invoked: target=" + target + " full=" + full);

    if (isCgiTarget_(full)) { 
        LOG_INFO("CGI selected");
        return cgi_.serve(req); 
    }
    LOG_INFO("Static selected (within FileOrCgiHandler)");
    return static_.serve(req);
}

bool FileOrCgiHandler::isCgiTarget_(const std::string& full) const {
    const OnOff& cgiExtension = docRoot_.getCgiExtensions();
    if (cgiExtension == OFF) {
        return false;
    }
    if (full.size() < 3) {
        return false;
    }
    const std::string lower = utils::toLower(full);
    if (lower.compare(lower.size() - 3, 3, ".py") != 0) {
        return false;
    }
    struct stat st;
    if (stat(full.c_str(), &st) != 0 || !S_ISREG(st.st_mode)) {
        return false;
    }
    if (access(full.c_str(), X_OK) != 0) {
        return false;
    }
    return true;
}

} // namespace http

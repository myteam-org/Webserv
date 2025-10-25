#include "action/cgi_action.hpp"

CgiActionPrepared::CgiActionPrepared(const PreparedCgi& preparedCgi) : preparedCgi_(preparedCgi){}

void CgiActionPrepared::execute(ActionContext &ctx) {
    (void)ctx;
}


#include "io/handler/CgiStdoutHandler.hpp"

CgiStdoutHandler::CgiStdoutHandler(Server* s) : srv_(s) {};

void CgiStdoutHandler::onEvent(const FdEntry& e, uint32_t m) {};

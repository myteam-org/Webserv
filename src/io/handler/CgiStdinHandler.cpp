#include "io/handler/CgiStdinHandler.hpp"

CgiStdinHandler::CgiStdinHandler(Server* s) : srv_(s) {};

void CgiStdinHandler::onEvent(const FdEntry& e, uint32_t m) {};
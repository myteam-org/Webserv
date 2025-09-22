#pragma once
#include "io/handler/IFdHandler.hpp"
#include "server/Server.hpp"

class Server;

class CgiStdoutHandler : public IFdHandler {
public:
  explicit CgiStdoutHandler(Server* s);
  void onEvent(const FdEntry& e, uint32_t m);
private: Server* srv_;
};

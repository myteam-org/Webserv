#pragma once
#include "io/handler/IFdHandler.hpp"
#include "server/Server.hpp"

class CgiStdinHandler : public IFdHandler {
public:
  explicit CgiStdinHandler(Server* s);
  void onEvent(const FdEntry& e, uint32_t m);
private: Server* srv_;
};
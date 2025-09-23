#pragma once
#include "io/handler/IFdHandler.hpp"
#include "server/Server.hpp"

class Server;

class ListenerHandler : public IFdHandler {
public:
  explicit ListenerHandler(Server* s);
  void onEvent(const FdEntry& e, uint32_t m);
private: Server* srv_;
};

#pragma once
#include <cstdint>
#include "server/fileDescriptor/FdRegistry.hpp"

class IFdHandler {
public:
  virtual ~IFdHandler() {}
  virtual void onEvent(const FdEntry& e, uint32_t mask) = 0;
};
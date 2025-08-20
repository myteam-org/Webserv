#pragma once
#include "utils.hpp"

class IConnectionState {
public:
    virtual types::Result<IConnectionState*, int>onEvent() =0;
    virtual ~IConnectionState() {};
};
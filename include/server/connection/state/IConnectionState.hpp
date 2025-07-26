
#include "utils.hpp"

class IConnectionState {
    virtual types::Result<IConnectionState, int>onEvent() =0;
};
#pragma once
#include "action/context.hpp"

class IAction {
public:
    virtual ~IAction() {}
    virtual void execute(ActionContext &ctx) = 0;
};

#pragma once

#include "sandbox/icons.h"

class Panel {
public:
    virtual ~Panel() = default;
    virtual void OnUIRender() = 0;
};
#pragma once

class Panel {
public:
    virtual ~Panel() = default;
    virtual void OnUIRender() = 0;
};
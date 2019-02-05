#pragma once

#include "Components.hpp"

class System {
public:
    virtual void Update(
        Components& components,
        size_t tick
    ) = 0;
};

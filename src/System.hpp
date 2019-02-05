#pragma once

#include "Components.hpp"

class System {
public:
    virtual void Update(Components& components) = 0;
};

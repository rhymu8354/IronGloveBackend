#pragma once

#include "../Component.hpp"

#include <string>

struct Tile : public Component {
    std::string name;
    int z = 0;
    int phase = 0;
    bool spinning = false;
    bool dirty = true;
    bool destroyed = false;
};

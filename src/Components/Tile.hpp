#pragma once

#include "../Component.hpp"

#include <string>

struct Tile : public Component {
    std::string name;
    int z = 0;
};

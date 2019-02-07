#pragma once

#include "../Component.hpp"

#include <string>

struct Weapon : public Component {
    int dx = 0;
    int dy = 0;
    int ownerId = 0;
};

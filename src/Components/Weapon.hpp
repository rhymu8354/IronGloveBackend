#pragma once

#include "../Component.hpp"

#include <string>

struct Weapon : public Component {
    int phase = 0;
    int dx = 0;
    int dy = 0;
};

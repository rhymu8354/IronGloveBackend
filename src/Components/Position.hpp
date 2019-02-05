#pragma once

#include "../Component.hpp"

struct Position : public Component {
    unsigned int x = 0;
    unsigned int y = 0;
};

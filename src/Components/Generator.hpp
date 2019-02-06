#pragma once

#include "../Component.hpp"

#include <string>
#include <WebSockets/WebSocket.hpp>

struct Generator : public Component {
    double spawnChance = 0.1;
};

#pragma once

#include "../Component.hpp"

#include <string>
#include <WebSockets/WebSocket.hpp>

struct Input : public Component {
    char fire = 0;
    bool fireReleased = false;
    bool fireThisTick = false;
    char move = 0;
    bool moveReleased = false;
    bool moveThisTick = false;
    bool weaponInFlight = false;
    int moveCooldown = 0;
};

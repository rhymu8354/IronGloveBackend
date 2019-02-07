#pragma once

#include "../Component.hpp"

#include <string>
#include <WebSockets/WebSocket.hpp>

struct Hero : public Component {
    int score = 0;
    int potions = 0;
};

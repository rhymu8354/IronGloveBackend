#pragma once

#include "../Component.hpp"

#include <string>
#include <WebSockets/WebSocket.hpp>

struct Health : public Component {
    int hp = 0;
};

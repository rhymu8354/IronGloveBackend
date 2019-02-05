#pragma once

#include "../Component.hpp"

#include <string>
#include <WebSockets/WebSocket.hpp>

struct Input : public Component {
    char fire = 0;
    char move = 0;
};

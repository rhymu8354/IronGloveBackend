#pragma once

#include "../Component.hpp"

#include <string>
#include <WebSockets/WebSocket.hpp>

struct Reward : public Component {
    int score = 0;
};

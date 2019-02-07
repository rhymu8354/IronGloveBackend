#pragma once

#include "../Component.hpp"

#include <string>
#include <WebSockets/WebSocket.hpp>

struct Pickup : public Component {
    enum class Type {
        Food,
        Potion,
        Treasure,
    } type = Type::Treasure;
};

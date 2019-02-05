#include "AI.hpp"

#include <Json/Value.hpp>
#include <memory>
#include <WebSockets/WebSocket.hpp>

struct AI::Impl {
    std::shared_ptr< WebSockets::WebSocket > ws;
};

AI::~AI() = default;

AI::AI()
    : impl_(new Impl())
{
}

void AI::Update(
    Components& components,
    size_t tick
) {
    if ((tick % 2) == 0) {
        return;
    }
    const auto inputsInfo = components.GetComponentsOfType(Components::Type::Input);
    if (inputsInfo.n != 1) {
        return;
    }
    const auto playerPosition = (Position*)components.GetEntityComponentOfType(Components::Type::Position, inputsInfo.first[0].entityId);
//    const auto positionsInfo = components.GetComponentsOfType(Components::Type::Position);
    const auto monstersInfo = components.GetComponentsOfType(Components::Type::Monster);
    auto monsters = (Monster*)monstersInfo.first;
    for (size_t i = 0; i < monstersInfo.n; ++i) {
        const auto& monster = monsters[i];
        const auto position = (Position*)components.GetEntityComponentOfType(Components::Type::Position, monster.entityId);
        if (position == nullptr) {
            continue;
        }
        const auto dx = abs(position->x - playerPosition->x);
        const auto dy = abs(position->y - playerPosition->y);
        if (dx > dy) {
            if (position->x < playerPosition->x) {
                ++position->x;
            } else if (position->x > playerPosition->x) {
                --position->x;
            }
        } else {
            if (position->y < playerPosition->y) {
                ++position->y;
            } else if (position->y > playerPosition->y) {
                --position->y;
            }
        }
    }
}

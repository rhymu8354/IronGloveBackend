#include "PlayerFiring.hpp"

#include <Json/Value.hpp>
#include <memory>
#include <WebSockets/WebSocket.hpp>

struct PlayerFiring::Impl {
    std::shared_ptr< WebSockets::WebSocket > ws;
};

PlayerFiring::~PlayerFiring() = default;

PlayerFiring::PlayerFiring()
    : impl_(new Impl())
{
}

void PlayerFiring::Update(
    Components& components,
    size_t tick
) {
    const auto inputsInfo = components.GetComponentsOfType(Components::Type::Input);
    auto inputs = (Input*)inputsInfo.first;
    for (size_t i = 0; i < inputsInfo.n; ++i) {
        auto& input = inputs[i];
        if (!input.fire) {
            continue;
        }
        const auto playerPosition = (Position*)components.GetEntityComponentOfType(Components::Type::Position, input.entityId);
        if (playerPosition == nullptr) {
            continue;
        }
        int dx = 0;
        int dy = 0;
        switch (input.fire) {
            case 'a': {
                dx = -1;
            } break;

            case 'd': {
                dx = 1;
            } break;

            case 'w': {
                dy = -1;
            } break;

            case 's': {
                dy = 1;
            } break;

            default: {
            }
        }
        if (input.fireReleased) {
            input.fire = 0;
        }
        if (
            !components.IsObstacleInTheWay(
                playerPosition->x + dx,
                playerPosition->y + dy
            )
        ) {
            const auto id = components.CreateEntity();
            const auto weapon = (Weapon*)components.CreateComponentOfType(Components::Type::Weapon, id);
            const auto weaponPosition = (Position*)components.CreateComponentOfType(Components::Type::Position, id);
            const auto tile = (Tile*)components.CreateComponentOfType(Components::Type::Tile, id);
            weapon->dx = dx;
            weapon->dy = dy;
            tile->name = "axe0";
            tile->z = 2;
            weaponPosition->x = playerPosition->x + dx;
            weaponPosition->y = playerPosition->y + dy;
        }
    }
}

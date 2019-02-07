#include "PlayerFiring.hpp"

#include <Json/Value.hpp>
#include <memory>
#include <set>
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
        const auto hero = (Hero*)components.GetEntityComponentOfType(
            Components::Type::Hero,
            input.entityId
        );
        if (hero == nullptr) {
            continue;
        }
        const auto playerPosition = (Position*)components.GetEntityComponentOfType(Components::Type::Position, input.entityId);
        if (playerPosition == nullptr) {
            continue;
        }
        if (
            input.usePotion
            && (hero->potions > 0)
        ) {
            --hero->potions;
            std::set< int > entitiesDestroyed;
            const auto monstersInfo = components.GetComponentsOfType(Components::Type::Monster);
            auto monsters = (Monster*)monstersInfo.first;
            for (size_t i = 0; i < monstersInfo.n; ++i) {
                const auto monsterPosition = (Position*)components.GetEntityComponentOfType(
                    Components::Type::Position,
                    monsters[i].entityId
                );
                if (monsterPosition == nullptr) {
                    continue;
                }
                const int dx = (monsterPosition->x - playerPosition->x);
                const int dy = (monsterPosition->y - playerPosition->y);
                if (sqrt((dx * dx) + (dy * dy)) <= 5) {
                    (void)entitiesDestroyed.insert(monsters[i].entityId);
                }
            }
            for (const auto entityId: entitiesDestroyed) {
                components.DestroyEntity(entityId);
            }
        }
        input.usePotion = false;
        if (!input.fire) {
            continue;
        }
        if (input.weaponInFlight) {
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
        input.fireThisTick = false;
        if (input.fireReleased) {
            input.fire = 0;
        }
        const auto id = components.CreateEntity();
        const auto weapon = (Weapon*)components.CreateComponentOfType(Components::Type::Weapon, id);
        const auto weaponPosition = (Position*)components.CreateComponentOfType(Components::Type::Position, id);
        const auto tile = (Tile*)components.CreateComponentOfType(Components::Type::Tile, id);
        weapon->dx = dx;
        weapon->dy = dy;
        tile->name = "axe";
        tile->z = 2;
        weaponPosition->x = playerPosition->x + dx;
        weaponPosition->y = playerPosition->y + dy;
        weapon->ownerId = input.entityId;
        input.weaponInFlight = true;
    }
}

#include "AI.hpp"

#include <Json/Value.hpp>
#include <set>
#include <memory>
#include <WebSockets/WebSocket.hpp>

namespace {

}

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
    if ((tick % 5) != 0) {
        return;
    }
    const auto inputsInfo = components.GetComponentsOfType(Components::Type::Input);
    if (inputsInfo.n != 1) {
        return;
    }
    const auto playerPosition = (Position*)components.GetEntityComponentOfType(Components::Type::Position, inputsInfo.first[0].entityId);
    if (playerPosition == nullptr) {
        return;
    }
    const auto playerHealth = (Health*)components.GetEntityComponentOfType(Components::Type::Health, playerPosition->entityId);
    const auto collidersInfo = components.GetComponentsOfType(Components::Type::Collider);
    const auto monstersInfo = components.GetComponentsOfType(Components::Type::Monster);
    auto monsters = (Monster*)monstersInfo.first;
    std::set< int > entitiesDestroyed;
    bool playerDestroyed = false;
    for (size_t i = 0; i < monstersInfo.n; ++i) {
        const auto& monster = monsters[i];
        const auto position = (Position*)components.GetEntityComponentOfType(Components::Type::Position, monster.entityId);
        if (position == nullptr) {
            continue;
        }
        const auto collider = (Collider*)components.GetEntityComponentOfType(Components::Type::Collider, monster.entityId);
        const auto mask = ((collider == nullptr) ? 0 : collider->mask);
        const auto dx = abs(position->x - playerPosition->x);
        const auto dy = abs(position->y - playerPosition->y);
        int mx = 0;
        int my = 0;
        if (position->x < playerPosition->x) {
            ++mx;
        } else if (position->x > playerPosition->x) {
            --mx;
        }
        if (position->y < playerPosition->y) {
            ++my;
        } else if (position->y > playerPosition->y) {
            --my;
        }
        if (
            (
                (position->x + mx == playerPosition->x)
                && (position->y == playerPosition->y)
            )
            || (
                (position->x == playerPosition->x)
                && (position->y + my == playerPosition->y)
            )
        ) {
            if (playerHealth != nullptr) {
                if (!playerDestroyed) {
                    playerHealth->hp -= 10;
                    if (playerHealth->hp <= 0) {
                        playerDestroyed = true;
                    }
                }
            }
            const auto monsterHealth = (Health*)components.GetEntityComponentOfType(Components::Type::Health, monster.entityId);
            if (monsterHealth != nullptr) {
                monsterHealth->hp = 0;
                (void)entitiesDestroyed.insert(monster.entityId);
            }
            continue;
        }
        if (
            (dx > dy)
            && !components.IsObstacleInTheWay(
                position->x + mx,
                position->y,
                mask
            )
        ) {
            position->x += mx;
        } else if (
            !components.IsObstacleInTheWay(
                position->x,
                position->y + my,
                mask
            )
        ) {
            position->y += my;
        } else if (
            !components.IsObstacleInTheWay(
                position->x + mx,
                position->y,
                mask
            )
        ) {
            position->x += mx;
        }
    }
    for (const auto entityId: entitiesDestroyed) {
        components.DestroyEntity(entityId);
    }
    if (playerDestroyed) {
        components.DestroyEntityComponentOfType(
            Components::Type::Position,
            playerPosition->entityId
        );
    }
}

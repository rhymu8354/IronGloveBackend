#include "PickupSystem.hpp"

#include <Json/Value.hpp>
#include <memory>
#include <WebSockets/WebSocket.hpp>
#include <SystemAbstractions/StringExtensions.hpp>

struct PickupSystem::Impl {
};

PickupSystem::~PickupSystem() = default;

PickupSystem::PickupSystem()
    : impl_(new Impl())
{
}

void PickupSystem::Update(
    Components& components,
    size_t tick
) {
    const auto heroesInfo = components.GetComponentsOfType(Components::Type::Hero);
    if (heroesInfo.n != 1) {
        return;
    }
    auto& hero = ((Hero*)heroesInfo.first)[0];
    const auto playerPosition = (Position*)components.GetEntityComponentOfType(
        Components::Type::Position,
        hero.entityId
    );
    const auto playerHealth = (Health*)components.GetEntityComponentOfType(
        Components::Type::Health,
        hero.entityId
    );
    if (
        (playerPosition == nullptr)
        || (playerHealth == nullptr)
    ) {
        return;
    }
    const auto pickupsInfo = components.GetComponentsOfType(Components::Type::Pickup);
    auto pickups = (Pickup*)pickupsInfo.first;
    std::set< int > entitiesDestroyed;
    bool exited = false;
    for (size_t i = 0; i < pickupsInfo.n; ++i) {
        auto& pickup = pickups[i];
        const auto position = (Position*)components.GetEntityComponentOfType(
            Components::Type::Position,
            pickup.entityId
        );
        if (position == nullptr) {
            continue;
        }
        if (
            (position->x == playerPosition->x)
            && (position->y == playerPosition->y)
        ) {
            switch (pickup.type) {
                case Pickup::Type::Treasure: {
                    hero.score += 100;
                    (void)entitiesDestroyed.insert(pickup.entityId);
                } break;
                case Pickup::Type::Food: {
                    playerHealth->hp += 100;
                    (void)entitiesDestroyed.insert(pickup.entityId);
                } break;
                case Pickup::Type::Potion: {
                    ++hero.potions;
                    (void)entitiesDestroyed.insert(pickup.entityId);
                } break;
                case Pickup::Type::Exit: {
                    exited = true;
                } break;
            }
        }
    }
    for (const auto entityId: entitiesDestroyed) {
        components.DestroyEntity(entityId);
    }
    if (exited) {
        components.DestroyEntityComponentOfType(
            Components::Type::Position,
            hero.entityId
        );
    }
}

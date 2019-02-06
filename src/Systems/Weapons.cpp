#include "Weapons.hpp"

#include <Json/Value.hpp>
#include <memory>
#include <WebSockets/WebSocket.hpp>
#include <SystemAbstractions/StringExtensions.hpp>

struct Weapons::Impl {
    std::shared_ptr< WebSockets::WebSocket > ws;
};

Weapons::~Weapons() = default;

Weapons::Weapons()
    : impl_(new Impl())
{
}

void Weapons::Update(
    Components& components,
    size_t tick
) {
    const auto weaponsInfo = components.GetComponentsOfType(Components::Type::Weapon);
    auto weapons = (Weapon*)weaponsInfo.first;
    std::set< int > entitiesDestroyed;
    for (size_t i = 0; i < weaponsInfo.n; ++i) {
        auto& weapon = weapons[i];
        const auto position = (Position*)components.GetEntityComponentOfType(Components::Type::Position, weapon.entityId);
        const auto tile = (Tile*)components.GetEntityComponentOfType(Components::Type::Tile, weapon.entityId);
        if (
            (position == nullptr)
            || (tile == nullptr)
        ) {
            continue;
        }
        weapon.phase = ((weapon.phase + 1) % 4);
        tile->name = SystemAbstractions::sprintf("axe%d", weapon.phase);
        auto collider = components.GetColliderAt(position->x, position->y);
        if (collider) {
            const auto health = (Health*)components.GetEntityComponentOfType(Components::Type::Health, collider->entityId);
            if (health != nullptr) {
                --health->hp;
                if (health->hp <= 0) {
                    (void)entitiesDestroyed.insert(collider->entityId);
                }
            }
            (void)entitiesDestroyed.insert(weapon.entityId);
        } else {
            const auto x = position->x + weapon.dx;
            const auto y = position->y + weapon.dy;
            collider = components.GetColliderAt(x, y);
            if (collider) {
                const auto health = (Health*)components.GetEntityComponentOfType(Components::Type::Health, collider->entityId);
                if (health != nullptr) {
                    --health->hp;
                    if (health->hp <= 0) {
                        (void)entitiesDestroyed.insert(collider->entityId);
                    }
                }
                (void)entitiesDestroyed.insert(weapon.entityId);
            } else {
                position->x = x;
                position->y = y;
            }
        }
    }
    for (const auto entityId: entitiesDestroyed) {
        components.DestroyEntity(entityId);
    }
}

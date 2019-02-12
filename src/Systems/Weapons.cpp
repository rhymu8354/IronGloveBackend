#include "Weapons.hpp"

#include <Json/Value.hpp>
#include <memory>
#include <WebSockets/WebSocket.hpp>
#include <SystemAbstractions/StringExtensions.hpp>

struct Weapons::Impl {
    std::shared_ptr< WebSockets::WebSocket > ws;

    void OnStrike(
        Components& components,
        const Weapon& weapon,
        const Collider& victimCollider,
        std::set< int >& entitiesDestroyed
    ) {
        const auto ownerInput = (Input*)components.GetEntityComponentOfType(
            Components::Type::Input,
            weapon.ownerId
        );
        const auto ownerHero = (Hero*)components.GetEntityComponentOfType(
            Components::Type::Hero,
            weapon.ownerId
        );
        const auto health = (Health*)components.GetEntityComponentOfType(
            Components::Type::Health,
            victimCollider.entityId
        );
        const auto reward = (Reward*)components.GetEntityComponentOfType(
            Components::Type::Reward,
            victimCollider.entityId
        );
        if (health != nullptr) {
            --health->hp;
            if (health->hp <= 0) {
                (void)entitiesDestroyed.insert(victimCollider.entityId);
                if (
                    (ownerHero != nullptr)
                    && (reward != nullptr)
                ) {
                    ownerHero->score += reward->score;
                }
            }
        }
        (void)entitiesDestroyed.insert(weapon.entityId);
        if (ownerInput != nullptr) {
            ownerInput->weaponInFlight = false;
        }
    }
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
        if (position == nullptr) {
            continue;
        }
        const auto tile = (Tile*)components.GetEntityComponentOfType(Components::Type::Tile, weapon.entityId);
        if (tile != nullptr) {
            tile->phase = ((tile->phase + 1) % 4);
        }
        auto collider = components.GetColliderAt(position->x, position->y);
        if (collider) {
            impl_->OnStrike(components, weapon, *collider, entitiesDestroyed);
        } else {
            const auto x = position->x + weapon.dx;
            const auto y = position->y + weapon.dy;
            collider = components.GetColliderAt(x, y);
            if (collider) {
                impl_->OnStrike(components, weapon, *collider, entitiesDestroyed);
            } else {
                position->x = x;
                position->y = y;
                if (tile != nullptr) {
                    tile->dirty = true;
                }
            }
        }
    }
    for (const auto entityId: entitiesDestroyed) {
        components.KillEntity(entityId);
    }
}

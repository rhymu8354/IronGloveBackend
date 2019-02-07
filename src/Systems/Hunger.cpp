#include "Hunger.hpp"

#include <Json/Value.hpp>
#include <set>
#include <memory>
#include <WebSockets/WebSocket.hpp>

struct Hunger::Impl {
    std::shared_ptr< WebSockets::WebSocket > ws;
};

Hunger::~Hunger() = default;

Hunger::Hunger()
    : impl_(new Impl())
{
}

void Hunger::Update(
    Components& components,
    size_t tick
) {
    if ((tick % 10) != 0) {
        return;
    }
    const auto heroesInfo = components.GetComponentsOfType(Components::Type::Hero);
    auto heroes = (Hero*)heroesInfo.first;
    std::set< int > entitiesStarved;
    for (size_t i = 0; i < heroesInfo.n; ++i) {
        auto& hero = heroes[i];
        const auto health = (Health*)components.GetEntityComponentOfType(
            Components::Type::Health,
            hero.entityId
        );
        const auto position = (Position*)components.GetEntityComponentOfType(
            Components::Type::Position,
            hero.entityId
        );
        if (
            (health == nullptr)
            || (position == nullptr)
        ) {
            continue;
        }
        --health->hp;
        if (health->hp <= 0) {
            (void)entitiesStarved.insert(hero.entityId);
        }
    }
    for (const auto entityId: entitiesStarved) {
        components.DestroyEntityComponentOfType(
            Components::Type::Position,
            entityId
        );
    }
}

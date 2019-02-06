#include "Generation.hpp"

#include <Json/Value.hpp>
#include <random>
#include <set>
#include <time.h>
#include <memory>
#include <WebSockets/WebSocket.hpp>

namespace {

    void AddMonster(
        Components& components,
        unsigned int x,
        unsigned int y
    ) {
        const auto id = components.CreateEntity();
        const auto collider = (Collider*)components.CreateComponentOfType(Components::Type::Collider, id);
        const auto health = (Health*)components.CreateComponentOfType(Components::Type::Health, id);
        const auto monster = (Monster*)components.CreateComponentOfType(Components::Type::Monster, id);
        const auto position = (Position*)components.CreateComponentOfType(Components::Type::Position, id);
        const auto tile = (Tile*)components.CreateComponentOfType(Components::Type::Tile, id);
        tile->name = "monster";
        tile->z = 1;
        position->x = x;
        position->y = y;
        health->hp = 1;
    }

}

struct Generation::Impl {
    std::shared_ptr< WebSockets::WebSocket > ws;
    std::mt19937 rng;
};

Generation::~Generation() = default;

Generation::Generation()
    : impl_(new Impl())
{
    impl_->rng.seed((int)time(NULL));
}

void Generation::Update(
    Components& components,
    size_t tick
) {
    const auto generatorsInfo = components.GetComponentsOfType(Components::Type::Generator);
    auto generators = (Generator*)generatorsInfo.first;
    for (size_t i = 0; i < generatorsInfo.n; ++i) {
        const auto& generator = generators[i];
        const auto position = (Position*)components.GetEntityComponentOfType(Components::Type::Position, generator.entityId);
        if (position == nullptr) {
            continue;
        }
        const auto roll = std::uniform_real_distribution< double >(0.0, 1.0)(impl_->rng);
        if (roll < generator.spawnChance) {
            const auto d = std::uniform_int_distribution< int >(0, 3)(impl_->rng);
            int x = position->x;
            int y = position->y;
            if (d / 2 == 0) {
                x += 2 * (d % 2) - 1;
            } else {
                y += 2 * (d % 2) - 1;
            }
            if (!components.IsObstacleInTheWay(x, y)) {
                AddMonster(components, x, y);
            }
        }
    }
}

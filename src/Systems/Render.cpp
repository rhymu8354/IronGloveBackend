#include "Render.hpp"

#include <Json/Value.hpp>
#include <memory>
#include <set>
#include <WebSockets/WebSocket.hpp>

struct Render::Impl {
    std::shared_ptr< WebSockets::WebSocket > ws;
    Json::Value previousRender;
};

Render::~Render() = default;

Render::Render()
    : impl_(new Impl())
{
}

void Render::SetClient(std::shared_ptr< WebSockets::WebSocket > ws) {
    impl_->ws = ws;
}

void Render::Update(
    Components& components,
    size_t tick
) {
    auto message = Json::Object({
        {"type", "render"},
    });
    auto sprites = Json::Array({});
    const auto tilesInfo = components.GetComponentsOfType(Components::Type::Tile);
    auto tiles = (Tile*)tilesInfo.first;
    std::set< int > entitiesDestroyed;
    for (size_t i = 0; i < tilesInfo.n; ++i) {
        auto& tile = tiles[i];
        auto sprite = Json::Object({
            {"id", tile.entityId},
        });
        if (tile.destroyed) {
            sprite["destroyed"] = true;
            (void)entitiesDestroyed.insert(tile.entityId);
        } else {
            if (!tile.dirty) {
                continue;
            }
            tile.dirty = false;
            const auto position = (Position*)components.GetEntityComponentOfType(Components::Type::Position, tile.entityId);
            if (position == nullptr) {
                continue;
            }
            sprite["texture"] = tile.name;
            sprite["x"] = (int)position->x;
            sprite["y"] = (int)position->y;
            sprite["z"] = tile.z;
            sprite["phase"] = tile.phase;
            sprite["spinning"] = tile.spinning;
            const auto weapon = (Weapon*)components.GetEntityComponentOfType(Components::Type::Weapon, tile.entityId);
            if (weapon != nullptr) {
                sprite["motion"] = Json::Object({
                    {"dx", weapon->dx},
                    {"dy", weapon->dy},
                });
            }
        }
        sprites.Add(std::move(sprite));
    }
    message["sprites"] = std::move(sprites);
    const auto heroesInfo = components.GetComponentsOfType(Components::Type::Hero);
    if (heroesInfo.n == 1) {
        const auto playerHealth = (Health*)components.GetEntityComponentOfType(Components::Type::Health, heroesInfo.first[0].entityId);
        const auto playerHero = (Hero*)&heroesInfo.first[0];
        message["health"] = playerHealth->hp;
        message["score"] = playerHero->score;
        message["potions"] = playerHero->potions;
    }
    if (message != impl_->previousRender) {
        impl_->ws->SendText(message.ToEncoding());
    }
    impl_->previousRender = message;
    for (const auto entityId: entitiesDestroyed) {
        components.DestroyEntityComponentOfType(Components::Type::Tile, entityId);
    }
}

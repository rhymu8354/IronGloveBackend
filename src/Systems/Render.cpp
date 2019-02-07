#include "Render.hpp"

#include <Json/Value.hpp>
#include <memory>
#include <WebSockets/WebSocket.hpp>

struct Render::Impl {
    std::shared_ptr< WebSockets::WebSocket > ws;
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
    for (size_t i = 0; i < tilesInfo.n; ++i) {
        const auto& tile = tiles[i];
        const auto position = (Position*)components.GetEntityComponentOfType(Components::Type::Position, tile.entityId);
        if (position == nullptr) {
            continue;
        }
        auto sprite = Json::Object({
            {"id", tile.entityId},
            {"texture", tile.name},
            {"x", (int)position->x},
            {"y", (int)position->y},
            {"z", tile.z},
            {"phase", tile.phase},
            {"spinning", tile.spinning},
        });
        const auto weapon = (Weapon*)components.GetEntityComponentOfType(Components::Type::Weapon, tile.entityId);
        if (weapon != nullptr) {
            sprite["motion"] = Json::Object({
                {"dx", weapon->dx},
                {"dy", weapon->dy},
            });
        }
        sprites.Add(std::move(sprite));
    }
    message["sprites"] = std::move(sprites);
    const auto inputsInfo = components.GetComponentsOfType(Components::Type::Input);
    if (inputsInfo.n == 1) {
        const auto playerHealth = (Health*)components.GetEntityComponentOfType(Components::Type::Health, inputsInfo.first[0].entityId);
        const auto playerHero = (Hero*)components.GetEntityComponentOfType(Components::Type::Hero, inputsInfo.first[0].entityId);
        message["health"] = playerHealth->hp;
        message["score"] = playerHero->score;
        message["potions"] = playerHero->potions;
    }
    impl_->ws->SendText(message.ToEncoding());
}

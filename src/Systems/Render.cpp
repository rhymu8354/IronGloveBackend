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
    auto sprites = Json::Array({});
    const auto tilesInfo = components.GetComponentsOfType(Components::Type::Tile);
    auto tiles = (Tile*)tilesInfo.first;
    for (size_t i = 0; i < tilesInfo.n; ++i) {
        const auto& tile = tiles[i];
        const auto position = (Position*)components.GetEntityComponentOfType(Components::Type::Position, tile.entityId);
        if (position == nullptr) {
            continue;
        }
        sprites.Add(Json::Object({
            {"id", tile.entityId},
            {"texture", tile.name},
            {"x", (int)position->x},
            {"y", (int)position->y},
        }));
    }
    impl_->ws->SendText(
        Json::Object({
            {"type", "render"},
            {"sprites", std::move(sprites)},
        }).ToEncoding()
    );
}

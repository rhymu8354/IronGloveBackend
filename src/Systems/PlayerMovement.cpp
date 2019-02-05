#include "PlayerMovement.hpp"

#include <Json/Value.hpp>
#include <memory>
#include <WebSockets/WebSocket.hpp>

struct PlayerMovement::Impl {
    std::shared_ptr< WebSockets::WebSocket > ws;
};

PlayerMovement::~PlayerMovement() = default;

PlayerMovement::PlayerMovement()
    : impl_(new Impl())
{
}

void PlayerMovement::Update(
    Components& components,
    size_t tick
) {
    const auto inputsInfo = components.GetComponentsOfType(Components::Type::Input);
    auto inputs = (Input*)inputsInfo.first;
    for (size_t i = 0; i < inputsInfo.n; ++i) {
        const auto& input = inputs[i];
        const auto position = (Position*)components.GetEntityComponentOfType(Components::Type::Position, input.entityId);
        if (position == nullptr) {
            continue;
        }
        switch (input.move) {
            case 'j': {
                if (position->x > 0) {
                    --position->x;
                }
            } break;

            case 'l': {
                if (position->x < 10) {
                    ++position->x;
                }
            } break;

            case 'i': {
                if (position->y > 0) {
                    --position->y;
                }
            } break;

            case 'k': {
                if (position->y < 10) {
                    ++position->y;
                }
            } break;

            default: {
            }
        }
    }
}

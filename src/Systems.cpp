#include "Systems.hpp"
#include "Systems/AI.hpp"
#include "Systems/Generation.hpp"
#include "Systems/PlayerMovement.hpp"
#include "Systems/Render.hpp"

SystemCollection Systems(
    std::shared_ptr< WebSockets::WebSocket > ws
) {
    const auto render = std::make_shared< Render >();
    render->SetClient(ws);
    return {
        std::make_shared< PlayerMovement >(),
        std::make_shared< AI >(),
        std::make_shared< Generation >(),
        render,
    };
}

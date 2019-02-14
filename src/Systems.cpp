#include "Systems.hpp"
#include "Systems/AI.hpp"
#include "Systems/PlayerFiring.hpp"
#include "Systems/Render.hpp"
#include "Systems/Weapons.hpp"

SystemCollection Systems(
    std::shared_ptr< WebSockets::WebSocket > ws
) {
    const auto render = std::make_shared< Render >();
    render->SetClient(ws);
    return {
        std::make_shared< Weapons >(),
        std::make_shared< PlayerFiring >(),
        std::make_shared< AI >(),
        render,
    };
}

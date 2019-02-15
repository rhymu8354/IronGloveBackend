#include "Systems.hpp"
#include "Systems/Render.hpp"

SystemCollection Systems(
    std::shared_ptr< WebSockets::WebSocket > ws
) {
    const auto render = std::make_shared< Render >();
    render->SetClient(ws);
    return {
        render,
    };
}

#pragma once

#include "System.hpp"

#include <memory>
#include <vector>
#include <WebSockets/WebSocket.hpp>

using SystemCollection = std::vector< std::shared_ptr< System > >;

SystemCollection Systems(
    std::shared_ptr< WebSockets::WebSocket > ws
);

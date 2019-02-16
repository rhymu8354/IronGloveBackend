#pragma once

/**
 * @file WebSocketWrapper.hpp
 *
 * This module declares the WebSocketWrapper structure.
 *
 * © 2019 by Richard Walters
 */

#include <memory>
#include <WebSockets/WebSocket.hpp>

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

struct WebSocketWrapper {
    /**
     * Link the class with the given Lua interpreter.
     *
     * @param[in] lua
     *     This points to the state of the Lua interpreter.
     */
    static void LinkLua(lua_State* lua);

    /**
     * Push the given WebSocket onto the Lua stack.
     *
     * @param[in] lua
     *     This points to the state of the Lua interpreter.
     *
     * @param[in] ws
     *     This is the WebSocket to push onto the Lua stack.
     */
    static void PushLua(
        lua_State* lua,
        std::shared_ptr< WebSockets::WebSocket > ws
    );
};

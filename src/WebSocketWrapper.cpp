/**
 * @file WebSocketWrapper.cpp
 *
 * This module contains the implementation of the WebSocketWrapper structure.
 *
 * © 2019 by Richard Walters
 */

#include "WebSocketWrapper.hpp"

#include <Json/Value.hpp>

namespace {

    /**
     * This is a Lua function registered as the __gc
     * object metamethod of the "json" class.
     *
     * @param[in] lua
     *     This points to the Lua interpreter instance.
     *
     * @return
     *     The number of values to return from the Lua stack is returned.
     */
    int Finalizer(lua_State* lua) {
        auto self = (std::shared_ptr< WebSockets::WebSocket >*)luaL_checkudata(lua, 1, "ws");
        self->~shared_ptr< WebSockets::WebSocket >();
        return 0;
    }

    /**
     * This is a Lua function registered as the SendText
     * object method of the "json" class.
     *
     * @param[in] lua
     *     This points to the Lua interpreter instance.
     *
     * @return
     *     The number of values to return from the Lua stack is returned.
     */
    int SendText(lua_State* lua) {
        auto self = (std::shared_ptr< WebSockets::WebSocket >*)luaL_checkudata(lua, 1, "ws");
        auto json = (Json::Value*)luaL_checkudata(lua, 2, "json");
        (*self)->SendText(json->ToEncoding());
        return 0;
    }

    /**
     * This is a Lua function registered as the __index
     * object metamethod of the "json" class.
     *
     * @param[in] lua
     *     This points to the Lua interpreter instance.
     *
     * @return
     *     The number of values to return from the Lua stack is returned.
     */
    int Index(lua_State* lua) {
        auto self = (std::shared_ptr< WebSockets::WebSocket >*)luaL_checkudata(lua, 1, "ws");
        const std::string fieldName = luaL_checkstring(lua, 2);
        if (fieldName == "SendText") {
            lua_pushcfunction(lua, SendText);
        } else {
            lua_pushnil(lua);
        }
        return 1;
    }

}

void WebSocketWrapper::LinkLua(lua_State* lua) {
    luaL_newmetatable(lua, "ws");
    lua_pushstring(lua, "__gc");
    lua_pushcfunction(lua, Finalizer);
    lua_settable(lua, -3);
    lua_pushstring(lua, "__index");
    lua_pushcfunction(lua, Index);
    lua_settable(lua, -3);
    lua_pushstring(lua, "SendText");
    lua_pushcfunction(lua, SendText);
    lua_pop(lua, 1);
}

void WebSocketWrapper::PushLua(
    lua_State* lua,
    std::shared_ptr< WebSockets::WebSocket > ws
) {
    auto self = (std::shared_ptr< WebSockets::WebSocket >*)lua_newuserdata(lua, sizeof(std::shared_ptr< WebSockets::WebSocket >));
    new (self) std::shared_ptr< WebSockets::WebSocket >(ws);
    luaL_setmetatable(lua, "ws");
}

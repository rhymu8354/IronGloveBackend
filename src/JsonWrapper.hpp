#pragma once

/**
 * @file JsonWrapper.hpp
 *
 * This module declares the JsonWrapper structure.
 *
 * © 2019 by Richard Walters
 */

#include <Json/Value.hpp>

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

/**
 * This is a Lua wrapper for a Json::Value object
 */
struct JsonWrapper {
    // Properties

    // Methods

    /**
     * Link the structure with the given Lua interpreter.
     *
     * @param[in] lua
     *     This points to the state of the Lua interpreter.
     */
    static void LinkLua(lua_State* lua);
};

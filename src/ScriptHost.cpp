/**
 * @file ScriptHost.cpp
 *
 * This module contains the implementation of the ScriptHost class.
 *
 * © 2019 by Richard Walters
 */

#include "Components.hpp"
#include "JsonWrapper.hpp"
#include "ScriptHost.hpp"
#include "WebSocketWrapper.hpp"

#include <stdlib.h>
#include <string>
#include <SystemAbstractions/StringExtensions.hpp>

extern "C" {
#include <luajit.h>
}

namespace {

    /**
     * This structure is used to pass state from the caller of the
     * lua_load function to the reader function supplied to lua_load.
     */
    struct LuaReaderState {
        /**
         * This is the code chunk to be read by the Lua interpreter.
         */
        const std::string* chunk = nullptr;

        /**
         * This flag indicates whether or not the Lua interpreter
         * has been fed the code chunk as input yet.
         */
        bool read = false;
    };

    /**
     * This function is provided to the Lua interpreter in order to
     * read the next chunk of code to be interpreted.
     *
     * @param[in] lua
     *     This points to the state of the Lua interpreter.
     *
     * @param[in] data
     *     This points to a LuaReaderState structure containing
     *     state information provided by the caller of lua_load.
     *
     * @param[out] size
     *     This points to where the size of the next chunk of code
     *     should be stored.
     *
     * @return
     *     A pointer to the next chunk of code to interpret is returned.
     *
     * @retval NULL
     *     This is returned once all the code to be interpreted has
     *     been read.
     */
    const char* LuaReader(lua_State* lua, void* data, size_t* size) {
        LuaReaderState* state = (LuaReaderState*)data;
        if (state->read) {
            return NULL;
        } else {
            state->read = true;
            *size = state->chunk->length();
            return state->chunk->c_str();
        }
    }

    /**
     * This function is provided to the Lua interpreter when lua_pcall
     * is called.  It is called by the Lua interpreter if a runtime
     * error occurs while interpreting scripts.
     *
     * @param[in] lua
     *     This points to the state of the Lua interpreter.
     *
     * @return
     *     The number of return values that have been pushed onto the
     *     Lua stack by the function as return values of the function
     *     is returned.
     */
    int LuaTraceback(lua_State* lua) {
        const char* message = lua_tostring(lua, 1);
        if (message == NULL) {
            if (!lua_isnoneornil(lua, 1)) {
                if (!luaL_callmeta(lua, 1, "__tostring")) {
                    lua_pushliteral(lua, "(no error message)");
                }
            }
        } else {
            luaL_traceback(lua, lua, message, 1);
        }
        return 1;
    }

}

struct ScriptHost::Impl {
    lua_State* lua = nullptr;

    Impl() {
        // Instantiate Lua interpreter.
        lua = luaL_newstate();

        // Crank up the performance OVER 9000!
//        luaJIT_setmode(lua, 0, LUAJIT_MODE_ENGINE|LUAJIT_MODE_ON);

        // Load standard Lua libraries.
        //
        // Temporarily disable the garbage collector as we load the
        // libraries, to improve performance
        // (http://lua-users.org/lists/lua-l/2008-07/msg00690.html).
        lua_gc(lua, LUA_GCSTOP, 0);
        luaL_openlibs(lua);
        lua_gc(lua, LUA_GCRESTART, 0);

        // Initialize wrapper types.
        Components::LinkLua(lua);
        JsonWrapper::LinkLua(lua);
        WebSocketWrapper::LinkLua(lua);
    }

    ~Impl() {
        lua_close(lua);
    }
};

ScriptHost::~ScriptHost() noexcept = default;

ScriptHost::ScriptHost()
    : impl_(new Impl())
{
}

lua_State* ScriptHost::GetLua() {
    return impl_->lua;
}

std::string ScriptHost::LoadScript(
    const std::string& name,
    const std::string& script
) {
    lua_settop(impl_->lua, 0);
    lua_pushcfunction(impl_->lua, LuaTraceback);
    LuaReaderState luaReaderState;
    luaReaderState.chunk = &script;
    std::string errorMessage;
    switch (const int luaLoadResult = lua_load(impl_->lua, LuaReader, &luaReaderState, ("=" + name).c_str())) {
        case 0: {
            const int luaPCallResult = lua_pcall(impl_->lua, 0, 0, 1);
            if (luaPCallResult != 0) {
                if (!lua_isnil(impl_->lua, -1)) {
                    errorMessage = lua_tostring(impl_->lua, -1);
                }
            }
        } break;
        case LUA_ERRSYNTAX: {
            errorMessage = lua_tostring(impl_->lua, -1);
        } break;
        case LUA_ERRMEM: {
            errorMessage = "LUA_ERRMEM";
        } break;
        default: {
            errorMessage = SystemAbstractions::sprintf("(unexpected lua_load result: %d)", luaLoadResult);
        } break;
    }
    lua_settop(impl_->lua, 0);
    return errorMessage;
}

std::string ScriptHost::Call(const std::string& luaFunctionName) {
    const int numberOfArguments = lua_gettop(impl_->lua);
    lua_pushcfunction(impl_->lua, LuaTraceback);
    lua_insert(impl_->lua, 1);
    lua_getglobal(impl_->lua, luaFunctionName.c_str());
    lua_insert(impl_->lua, 2);
    const int luaPCallResult = lua_pcall(impl_->lua, numberOfArguments, 0, 1);
    std::string errorMessage;
    if (luaPCallResult != 0) {
        if (!lua_isnil(impl_->lua, -1)) {
            errorMessage = lua_tostring(impl_->lua, -1);
        }
    }
    lua_settop(impl_->lua, 0);
    return errorMessage;
}

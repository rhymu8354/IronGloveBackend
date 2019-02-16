/**
 * @file JsonWrapper.cpp
 *
 * This module contains the implementation of the JsonWrapper structure.
 *
 * © 2019 by Richard Walters
 */

#include "JsonWrapper.hpp"

namespace {

    /**
     * Construct a new Json::Value based on a value on the Lua stack.
     *
     * @param[in] lua
     *     This points to the Lua interpreter instance.
     *
     * @param[in] index
     *     This is the index of the value on the Lua stack that will be wrapped.
     *
     * @return
     *     The newly constructed Json::Value is returned.
     */
    Json::Value JsonValueFromLuaValue(
        lua_State* lua,
        int index
    ) {
        luaL_checkany(lua, index);
        switch (lua_type(lua, index)) {
            case LUA_TNIL: {
                return Json::Value();
            } break;
            case LUA_TNUMBER: {
                if (lua_isinteger(lua, index)) {
                    return Json::Value((int)lua_tointeger(lua, index));
                } else {
                    return Json::Value((double)lua_tonumber(lua, index));
                }
            } break;
            case LUA_TBOOLEAN: {
                return Json::Value(lua_toboolean(lua, index) != 0);
            } break;
            case LUA_TSTRING: {
                return Json::Value(lua_tostring(lua, index));
            } break;
            case LUA_TUSERDATA: {
                void* udata = luaL_testudata(lua, index, "json");
                if (udata != nullptr) {
                    auto json = (Json::Value*)udata;
                    return *json;
                } else {
                    (void)luaL_error(lua, "cannot construct a JSON value from a %s", lua_typename(lua, lua_type(lua, index)));
                }
            }
            default: {
                (void)luaL_error(lua, "cannot construct a JSON value from a %s", lua_typename(lua, lua_type(lua, index)));
            }
        }
        return Json::Value();
    }

    /**
     * This is a Lua function registered as the __call
     * class metamethod of the "json" class.
     *
     * @param[in] lua
     *     This points to the Lua interpreter instance.
     */
    int Constructor(lua_State* lua) {
        const int numArgs = (int)lua_gettop(lua);
        auto self = (Json::Value*)lua_newuserdata(lua, sizeof(Json::Value));
        if (numArgs >= 2) {
            auto json = JsonValueFromLuaValue(lua, 2);
            new (self) Json::Value(std::move(json));
        } else {
            new (self) Json::Value();
        }
        luaL_setmetatable(lua, "json");
        return 1;
    }

    /**
     * This is a Lua function registered as the Add
     * class method of the "json" class.
     *
     * @param[in] lua
     *     This points to the Lua interpreter instance.
     */
    int Add(lua_State* lua) {
        auto self = (Json::Value*)luaL_checkudata(lua, 1, "json");
        auto json = JsonValueFromLuaValue(lua, 2);
        self->Add(json);
        return 0;
    }

    /**
     * This is a Lua function registered as the Parse
     * class method of the "json" class.
     *
     * @param[in] lua
     *     This points to the Lua interpreter instance.
     */
    int Parse(lua_State* lua) {
        const std::string encoding = luaL_checkstring(lua, 1);
        auto json = (Json::Value*)lua_newuserdata(lua, sizeof(Json::Value));
        new (json) Json::Value(Json::Value::FromEncoding(encoding));
        luaL_setmetatable(lua, "json");
        return 1;
    }

    /**
     * This is a Lua function registered as the __index
     * class metamethod of the "json" class.
     *
     * @param[in] lua
     *     This points to the Lua interpreter instance.
     */
    int ClassIndex(lua_State* lua) {
        const std::string fieldName = luaL_checkstring(lua, 2);
        if (fieldName == "Add") {
            lua_pushcfunction(lua, Add);
        } else if (fieldName == "Parse") {
            lua_pushcfunction(lua, Parse);
        } else {
            lua_pushnil(lua);
        }
        return 1;
    }

    /**
     * This is a Lua function registered as the __gc
     * object metamethod of the "json" class.
     *
     * @param[in] lua
     *     This points to the Lua interpreter instance.
     */
    int Finalizer(lua_State* lua) {
        auto self = (Json::Value*)luaL_checkudata(lua, 1, "json");
        self->~Value();
        return 0;
    }

    /**
     * This is a Lua function registered as the __index
     * object metamethod of the "json" class.
     *
     * @param[in] lua
     *     This points to the Lua interpreter instance.
     */
    int ObjectIndex(lua_State* lua) {
        auto self = (Json::Value*)luaL_checkudata(lua, 1, "json");
        const std::string fieldName = luaL_checkstring(lua, 2);
        const auto& value = self->operator[](fieldName);
        switch (value.GetType()) {
            case Json::Value::Type::Array:
            case Json::Value::Type::Object: {
                auto returnValue = (Json::Value*)lua_newuserdata(lua, sizeof(Json::Value));
                new (returnValue) Json::Value(value);
                luaL_setmetatable(lua, "json");
            } break;
            case Json::Value::Type::Boolean: {
                lua_pushboolean(lua, (bool)value ? 1 : 0);
            } break;
            case Json::Value::Type::FloatingPoint: {
                lua_pushnumber(lua, (lua_Number)(double)value);
            } break;
            case Json::Value::Type::Integer: {
                lua_pushinteger(lua, (lua_Integer)(int)value);
            } break;
            case Json::Value::Type::String: {
                const auto valueAsString = (std::string)value;
                lua_pushlstring(lua, valueAsString.c_str(), valueAsString.length());
            } break;
            case Json::Value::Type::Null:
            case Json::Value::Type::Invalid:
            default: {
                lua_pushnil(lua);
            }
        }
        return 1;
    }

    /**
     * This is a Lua function registered as the __newindex
     * object metamethod of the "json" class.
     *
     * @param[in] lua
     *     This points to the Lua interpreter instance.
     */
    int ObjectNewIndex(lua_State* lua) {
        auto self = (Json::Value*)luaL_checkudata(lua, 1, "json");
        const std::string fieldName = luaL_checkstring(lua, 2);
        auto json = JsonValueFromLuaValue(lua, 3);
        self->operator[](fieldName) = std::move(json);
        return 0;
    }

    /**
     * This is a Lua function registered as the __tostring
     * object metamethod of the "json" class.
     *
     * @param[in] lua
     *     This points to the Lua interpreter instance.
     */
    int ToString(lua_State* lua) {
        auto self = (Json::Value*)luaL_checkudata(lua, 1, "json");
        const std::string encoding = self->ToEncoding();
        lua_pushlstring(lua, encoding.c_str(), encoding.length());
        return 1;
    }

    /**
     * This is a Lua function registered as the __len
     * object metamethod of the "json" class.
     *
     * @param[in] lua
     *     This points to the Lua interpreter instance.
     */
    int Len(lua_State* lua) {
        auto self = (Json::Value*)luaL_checkudata(lua, 1, "json");
        lua_pushinteger(lua, self->GetSize());
        return 1;
    }

}

void JsonWrapper::LinkLua(lua_State* lua) {
    luaL_newmetatable(lua, "json");
    lua_pushvalue(lua, -1);
    lua_setglobal(lua, "json");
    lua_createtable(lua, 0, 1);
    lua_pushstring(lua, "__call");
    lua_pushcfunction(lua, Constructor);
    lua_settable(lua, -3);
    lua_pushstring(lua, "__index");
    lua_pushcfunction(lua, ClassIndex);
    lua_settable(lua, -3);
    lua_setmetatable(lua, -2);
    lua_pushstring(lua, "__gc");
    lua_pushcfunction(lua, Finalizer);
    lua_settable(lua, -3);
    lua_pushstring(lua, "__index");
    lua_pushcfunction(lua, ObjectIndex);
    lua_settable(lua, -3);
    lua_pushstring(lua, "__newindex");
    lua_pushcfunction(lua, ObjectNewIndex);
    lua_settable(lua, -3);
    lua_pushstring(lua, "__tostring");
    lua_pushcfunction(lua, ToString);
    lua_settable(lua, -3);
    lua_pushstring(lua, "__len");
    lua_pushcfunction(lua, Len);
    lua_settable(lua, -3);
    lua_pop(lua, 1);
}

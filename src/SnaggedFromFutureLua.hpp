#pragma once

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

void luaL_setmetatable (lua_State *L, const char *tname);
void *luaL_testudata (lua_State *L, int ud, const char *tname);

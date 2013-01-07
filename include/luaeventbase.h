#ifndef LUAEVENTBASE_H_
#define LUAEVENTBASE_H_

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

typedef struct tagluaeventbase
{
	lua_State* L;
	struct event_base* event_base;
}lua_event_base;

#define LUA_EVENTBASE_META "luaeventbase_meta"
#define LUA_EVENTBASE "luaeventbase"

LUALIB_API int luaopen_luaeventbase(lua_State* L);

#endif

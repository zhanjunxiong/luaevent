
#ifndef LUALISTENER_H_
#define LUALISTENER_H_

#include "luaeventbase.h"

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

typedef struct taglualistener
{
	lua_event_base* leb;
	struct evconnlistener* listener;
	int cb;
}lualistener;

#define LUA_LISTENER_META "lualistener_meta"
#define LUA_LISTENER "lualistener"

LUALIB_API int luaopen_lualistener(lua_State* L);

#endif

#ifndef LUAEVENT_H_
#define LUAEVENT_H_

#include "luaeventbase.h"

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

typedef struct tagluaevent
{
	lua_event_base* leb;
	struct event* event;
	int cb;
}lua_event;

#define LUA_EVENT_META "luaevent_meta"
#define LUA_EVENT "luaevent"

LUALIB_API int luaopen_luaevent(lua_State* L);

#endif

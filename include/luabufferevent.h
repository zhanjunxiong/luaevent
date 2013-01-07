
#ifndef LUABUFFEREVENT_H_
#define LUABUFFEREVENT_H_

#include "luaeventbase.h"

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

typedef struct tagluabufferevent
{
	lua_event_base* leb;
	struct bufferevent* bufferevent;
	int readcb;
	int writecb;
	int eventcb;
}lua_bufferevent;

#define LUA_BUFFEREVENT_META "luabufferevent_meta"
#define LUA_BUFFEREVENT "luabufferevent"

LUALIB_API int luaopen_bufferevent(lua_State* L);

#endif

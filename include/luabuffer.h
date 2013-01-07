
#ifndef LUABUFFER_H_
#define LUABUFFER_H_

#include "luaeventbase.h"

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

typedef struct tagluaevbuffer
{
	lua_event_base* leb;
	struct evbuffer* evbuf;
	int bfree;
}lua_evbuffer;

#define LUA_EVBUFFER_META "luaevbuffer_meta"
#define LUA_EVBUFFER "luaevbuffer"

LUALIB_API int luaopen_evbuffer(lua_State* L);

int push_evbuffer(lua_State* L, struct evbuffer* evbuf);

#endif


#include "luaeventbase.h"
#include "luabuffer.h"
#include "luabufferevent.h"
#include "luaevent.h"
#include "lualistener.h"
#include "luautil.h"

#include <event2/event.h>
#include <event2/util.h>

static int luaeventbase_new(lua_State* L)
{
	struct event_base* base = event_base_new();
	if (!base) return 0;

	lua_event_base* event = (lua_event_base*)(lua_newuserdata(L, sizeof(lua_event_base)));
	if (!event) return 0;

	event->event_base = base;
	event->L = L;
	luaL_getmetatable(L, LUA_EVENTBASE_META);
	lua_setmetatable(L, -2);
	return 1;
}

static int luaeventbase_free(lua_State* L)
{
	lua_event_base* event = (lua_event_base*)(luaL_checkudata(L, 1, LUA_EVENTBASE_META));

	event_base_free(event->event_base);
	return 0;
}

static int luaeventbase_dispatch(lua_State* L)
{
	lua_event_base* event = (lua_event_base*)(luaL_checkudata(L, 1, LUA_EVENTBASE_META));

	int ret = event_base_dispatch(event->event_base);
	lua_pushinteger(L, ret);
	return 1;
}

static int luaeventbase_loop(lua_State* L)
{
	lua_event_base* event = (lua_event_base*)(luaL_checkudata(L, 1, LUA_EVENTBASE_META));

	int flag = luaL_checknumber(L, 2);
	int ret = event_base_loop(event->event_base, flag);
	lua_pushinteger(L, ret);
	return 1;
}

static int luaeventbase_loopexit(lua_State* L)
{
	lua_event_base* event = (lua_event_base*)(luaL_checkudata(L, 1, LUA_EVENTBASE_META));

	long sec = luaL_checknumber(L, 2);
	long usec = luaL_checknumber(L, 3);
	struct  timeval tv;
	tv.tv_sec = sec;
	tv.tv_usec = usec;
	int ret = event_base_loopexit(event->event_base, &tv);
	lua_pushinteger(L, ret);
	return 1;
}

static int luaeventbase_loopbreak(lua_State* L)
{
	lua_event_base* event = (lua_event_base*)(luaL_checkudata(L, 1, LUA_EVENTBASE_META));

	int ret = event_base_loopbreak(event->event_base);
	lua_pushinteger(L, ret);
	return 1;
}

static int luaeventbase_got_exit(lua_State* L)
{
	lua_event_base* event = (lua_event_base*)(luaL_checkudata(L, 1, LUA_EVENTBASE_META));

	int ret = event_base_got_exit(event->event_base);
	lua_pushinteger(L, ret);
	return 1;
}

static int luaeventbase_got_break(lua_State* L)
{
	lua_event_base* event = (lua_event_base*)(luaL_checkudata(L, 1, LUA_EVENTBASE_META));

	int ret = event_base_got_break(event->event_base);
	lua_pushinteger(L, ret);
	return 1;
}

static int luaevent_get_method(lua_State* L)
{
	lua_event_base* event = (lua_event_base*)(luaL_checkudata(L, 1, LUA_EVENTBASE_META));

	const char* method = event_base_get_method(event->event_base);
	lua_pushstring(L, method);
	return 1;
}

static int luaevent_get_version(lua_State* L)
{
	const char* version = event_get_version();
	lua_pushstring(L, version);
	return 1;
}

static const luaL_Reg lib[] =
{
		{"new", luaeventbase_new},
		{NULL, NULL}
};

static const luaL_Reg libm[] =
{
		{"__gc", luaeventbase_free},
		{NULL, NULL}
};

static const luaL_Reg indexfunlib[] =
{
		{"dispatch", luaeventbase_dispatch},
		{"loop", luaeventbase_loop},
		{"loopexit", luaeventbase_loopexit},
		{"loopbreak", luaeventbase_loopbreak},
		{"got_exit", luaeventbase_got_exit},
		{"got_break", luaeventbase_got_break},
		{"get_method", luaevent_get_method },
		{"get_version", luaevent_get_version},
		{NULL, NULL}
};

static const name_integer consts[] =
{
		{"EVLOOP_ONCE", EVLOOP_ONCE},
		{"EVLOOP_NONBLOCK", EVLOOP_NONBLOCK},
		{NULL, 0}
};

int luaopen_luaeventbase(lua_State* L)
{
#ifdef WIN32
	WORD wVersionRequested;
	WSADATA wsaData;
	int	err;

	wVersionRequested = MAKEWORD(2, 2);
	err = WSAStartup(wVersionRequested, &wsaData);
#endif

	luaL_newmetatable(L, LUA_EVENTBASE_META);
	register_index_fun(L, indexfunlib);
	luaL_register(L, NULL, libm);

	//
	luaopen_evbuffer(L);
	lua_pop(L, 1);
	luaopen_bufferevent(L);
	lua_pop(L, 1);
	luaopen_luaevent(L);
	lua_pop(L, 1);
	luaopen_lualistener(L);
	lua_pop(L, 1);
	//
	luaL_register(L, LUA_EVENTBASE, lib);
	set_name_integers(L, consts);
	return 1;
}

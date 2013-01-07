
#include "luaevent.h"
#include "luautil.h"

#include <assert.h>

#include <event2/event.h>
#include <event2/util.h>

static void luaevent_event_cb(evutil_socket_t fd, short ev, void* arg)
{
	lua_event* event = (lua_event*)arg;
	if (!event) return;
	lua_event_base* leb = event->leb;
	if (!leb) return;
	lua_State* L = leb->L;
	if (!L) return;

	lua_rawgeti(L, LUA_REGISTRYINDEX, event->cb);
	if (lua_isfunction(L, -1))
	{
		lua_pushlightuserdata(L, (void*)fd);
		lua_pushinteger(L, ev);
		lua_pcall(L, 2, 0, 0);
	}
}

static int luaevent_new(lua_State* L)
{
	lua_event_base* leb = (lua_event_base*)(luaL_checkudata(L, 1, LUA_EVENTBASE_META));

	struct event_base* base = leb->event_base;
	assert(base);

	evutil_socket_t fd = get_evutil_socket_t(L, 2);
	short ev = luaL_checknumber(L, 3);
	luaL_checktype(L, 4, LUA_TFUNCTION);

	lua_event* luaevent = (lua_event*)(lua_newuserdata(L, sizeof(lua_event)));
	if (!luaevent) return 0;

	struct event* event = event_new(base, fd, ev, luaevent_event_cb, luaevent);
	if (!event) return 0;

	lua_pushvalue(L, 4);
	int cb = luaL_ref(L, LUA_REGISTRYINDEX);

	luaevent->event = event;
	luaevent->leb = leb;
	luaevent->cb = cb;
	luaL_getmetatable(L, LUA_EVENT_META);
	lua_setmetatable(L, -2);
	return 1;
}

static int luaevent_free(lua_State* L)
{
	lua_event* luaevent = (lua_event*)(luaL_checkudata(L, 1, LUA_EVENT_META));

	luaL_unref(L, LUA_REGISTRYINDEX, luaevent->cb);
	event_free(luaevent->event);
	return 0;
}

static int luaevent_add(lua_State* L)
{
	lua_event* luaevent = (lua_event*)(luaL_checkudata(L, 1, LUA_EVENT_META));

	struct event* ev = luaevent->event;
	int ret = -1;
	if (lua_gettop(L) == 3)
	{
		long sec = luaL_checknumber(L, 2);
		long usec = luaL_checknumber(L, 3);
		struct  timeval timeout;
		timeout.tv_sec = sec;
		timeout.tv_usec = usec;
		ret = event_add(ev, &timeout);
	}
	else
	{
		ret = event_add(ev, NULL);
	}

	lua_pushinteger(L, ret);
	return 1;
}

static int luaevent_del(lua_State* L)
{
	lua_event* luaevent = (lua_event*)(luaL_checkudata(L, 1, LUA_EVENT_META));

	struct event* ev = luaevent->event;
	int ret = event_del(ev);
	lua_pushinteger(L, ret);
	return 1;
}

static const luaL_Reg lib[] =
{
		{"new", luaevent_new},
		{NULL, NULL}
};

static const luaL_Reg libm[] =
{
		{"__gc", luaevent_free},
		{NULL, NULL}
};

static const luaL_Reg indexlib[] =
{
		{"add", luaevent_add},
		{"del", luaevent_del},
		{NULL, NULL}
};

static const name_integer consts[] =
{
		{"EV_TIMEOUT", EV_TIMEOUT},
		{"EV_READ", EV_READ},
		{"EV_WRITE", EV_WRITE},
		{"EV_SIGNAL", EV_SIGNAL},
		{"EV_PERSIST", EV_PERSIST},
		{"EV_ET", EV_ET},
		{NULL, 0}
};

LUALIB_API int luaopen_luaevent(lua_State* L)
{
	luaL_newmetatable(L, LUA_EVENT_META);
	register_index_fun(L, indexlib);
	luaL_register(L, NULL, libm);

	luaL_register(L, LUA_EVENT, lib);
	set_name_integers(L, consts);
	return 1;
}

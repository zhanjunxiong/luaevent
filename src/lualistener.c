
#include "lualistener.h"
#include "luautil.h"

#include <event2/util.h>
#include <event2/listener.h>

#include <string.h> // for memset

static void lualistener_cb(struct evconnlistener* evconnlistener, evutil_socket_t fd, struct sockaddr* addr, int socklen, void* arg)
{
	lualistener* listener = (lualistener*)arg;
	if (!listener) return;

	lua_event_base* leb = listener->leb;

	lua_State* L = leb->L;
	if (!L) return;

	lua_rawgeti(L, LUA_REGISTRYINDEX, listener->cb);
	if (lua_isfunction(L, -1))
	{
		lua_pushlightuserdata(L, (void*)fd);
		lua_pcall(L, 1, 0, 0);
	}
}

static int lualistener_new_bind(lua_State* L)
{
	lua_event_base* leb = (lua_event_base*)(luaL_checkudata(L, 1, LUA_EVENTBASE_META));

	luaL_checktype(L, 2, LUA_TFUNCTION);

	unsigned flags = luaL_checknumber(L, 3);
	int backlog = luaL_checknumber(L, 4);
	const char* str = luaL_checkstring(L, 5);
	struct sockaddr_storage listen_on_addr;
	memset(&listen_on_addr, 0, sizeof(listen_on_addr));
	int socklen = sizeof(listen_on_addr);
	int ret = evutil_parse_sockaddr_port(str, (struct sockaddr*)&listen_on_addr, &socklen);
	if (ret != 0)
	{
		fprintf(stderr, "lualistener_new_bind addr(%s) error!!!\n", str);
		return 0;
	}

	lua_pushvalue(L, 2);
	int cb = luaL_ref(L, LUA_REGISTRYINDEX);

	lualistener* listener = (lualistener*)(lua_newuserdata(L, sizeof(lualistener)));
	if (!listener) return 0;

	struct evconnlistener* evconnlistener = evconnlistener_new_bind(leb->event_base, lualistener_cb, listener, flags, backlog, (struct sockaddr*)&listen_on_addr, socklen);
	if (!evconnlistener) return 0;

	listener->leb = leb;
	listener->listener = evconnlistener;
	listener->cb = cb;

	luaL_getmetatable(L, "lualistener_meta");
	lua_setmetatable(L, -2);
	return 1;
}

static int lualistener_free(lua_State* L)
{
	lualistener* listener = (lualistener*)(luaL_checkudata(L, 1, LUA_LISTENER_META));

	luaL_unref(L, LUA_REGISTRYINDEX, listener->cb);
	evconnlistener_free(listener->listener);
	return 0;
}

static const luaL_Reg lib[] =
{
		{"new", lualistener_new_bind},
		{NULL, NULL}
};

static const luaL_Reg libm[] =
{
		{"__gc", lualistener_free},
		{NULL, NULL}
};

static const name_integer consts[] =
{
		{"LEV_OPT_LEAVE_SOCKETS_BLOCKING", LEV_OPT_LEAVE_SOCKETS_BLOCKING},
		{"LEV_OPT_CLOSE_ON_FREE", LEV_OPT_CLOSE_ON_FREE},
		{"LEV_OPT_CLOSE_ON_EXEC", LEV_OPT_CLOSE_ON_EXEC},
		{"LEV_OPT_REUSEABLE", LEV_OPT_REUSEABLE},
		{"LEV_OPT_THREADSAFE", LEV_OPT_THREADSAFE},
		{NULL, 0}
};

LUALIB_API int luaopen_lualistener(lua_State* L)
{
	luaL_newmetatable(L, LUA_LISTENER_META);
	luaL_register(L, NULL, libm);

	luaL_register(L, LUA_LISTENER, lib);
	set_name_integers(L, consts);
	return 1;
}


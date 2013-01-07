
#include "luabuffer.h"
#include "luautil.h"

#include <event2/buffer.h>

#include <stdlib.h> // for free

static int luaevbuffer_new(lua_State* L)
{
	lua_evbuffer* luaevbuf = (lua_evbuffer*)lua_newuserdata(L, sizeof(lua_evbuffer));
	if (!luaevbuf) return 0;

	struct evbuffer * evbuf = evbuffer_new();
	luaevbuf->evbuf = evbuf;
	luaevbuf->bfree = 1;
	luaL_getmetatable(L, LUA_EVBUFFER_META);
	lua_setmetatable(L, -2);
	return 1;
}

static int luaevbuffer_free(lua_State* L)
{
	lua_evbuffer* luaevbuf = (lua_evbuffer*)(luaL_checkudata(L, 1, LUA_EVBUFFER_META));

	if (luaevbuf->evbuf && luaevbuf->bfree == 1)
	{
		evbuffer_free(luaevbuf->evbuf);
		luaevbuf->evbuf = NULL;
	}
	return 0;
}

static int luaevbuffer_add(lua_State* L)
{
	lua_evbuffer* luaevbuf = (lua_evbuffer*)(luaL_checkudata(L, 1, LUA_EVBUFFER_META));

	size_t size;
	const char* data = luaL_checklstring(L, 2, &size);
	int ret = evbuffer_add(luaevbuf->evbuf, data, size);
	lua_pushinteger(L, ret);
	return 1;
}

static int luaevbuffer_get_length(lua_State* L)
{
	lua_evbuffer* luaevbuf = (lua_evbuffer*)(luaL_checkudata(L, 1, LUA_EVBUFFER_META));

	size_t len = evbuffer_get_length(luaevbuf->evbuf);
	lua_pushnumber(L, len);
	return 1;
}

static int luaevbuffer_readln(lua_State* L)
{
	lua_evbuffer* luaevbuf = (lua_evbuffer*)(luaL_checkudata(L, 1, LUA_EVBUFFER_META));

	int eol_style = luaL_checknumber(L, 2);
	size_t datalen;
	char* data = evbuffer_readln(luaevbuf->evbuf, &datalen, eol_style);
	if (!data)
	{
		return 0;
	}
	lua_pushlstring(L, data, datalen);
	free(data);
	return 1;
}

static int luaevbuffer_drain(lua_State* L)
{
	lua_evbuffer* luaevbuf = (lua_evbuffer*)(luaL_checkudata(L, 1, LUA_EVBUFFER_META));

	size_t len = luaL_checknumber(L, 2);
	int ret = evbuffer_drain(luaevbuf->evbuf, len);
	lua_pushinteger(L, ret);
	return 1;
}

static int luaevbuffer_read(lua_State* L)
{
	lua_evbuffer* luaevbuf = (lua_evbuffer*)(luaL_checkudata(L, 1, LUA_EVBUFFER_META));

	evutil_socket_t fd = get_evutil_socket_t(L, 2);
	int howmuch = luaL_checknumber(L, 3);
	int ret = evbuffer_read(luaevbuf->evbuf, fd, howmuch);
	lua_pushinteger(L, ret);
	return 1;
}

static int luaevbuffer_write(lua_State* L)
{
	lua_evbuffer* luaevbuf = (lua_evbuffer*)(luaL_checkudata(L, 1, LUA_EVBUFFER_META));

	evutil_socket_t fd = get_evutil_socket_t(L, 2);
	int howmuch = luaL_optnumber(L, 3, -1);
	int ret = evbuffer_write_atmost(luaevbuf->evbuf, fd, howmuch);
	lua_pushinteger(L, ret);
	return 1;
}

static const luaL_Reg lib[] =
{
		{"new", luaevbuffer_new},
		{NULL, NULL}
};

static const luaL_Reg libm[] =
{
		{"__gc", luaevbuffer_free},
		{NULL, NULL}
};


static const luaL_Reg indexlib[] =
{
		{"add", luaevbuffer_add},
		{"get_length", luaevbuffer_get_length},
		{"readln", luaevbuffer_readln},
		{"drain", luaevbuffer_drain},
		{"close", luaevbuffer_free},
		{"read", luaevbuffer_read},
		{"write", luaevbuffer_write},
		{NULL, NULL}
};

static const name_integer consts[] =
{
		{"EVBUFFER_EOL_ANY", EVBUFFER_EOL_ANY},
		{"EVBUFFER_EOL_CRLF", EVBUFFER_EOL_CRLF},
		{"EVBUFFER_EOL_CRLF_STRICT", EVBUFFER_EOL_CRLF_STRICT},
		{"EVBUFFER_EOL_LF", EVBUFFER_EOL_LF},
		{NULL, 0}
};

LUALIB_API int luaopen_evbuffer(lua_State* L)
{
	luaL_newmetatable(L, LUA_EVBUFFER_META);
	register_index_fun(L, indexlib);
	luaL_register(L, NULL, libm);

	luaL_register(L, LUA_EVBUFFER, lib);
	set_name_integers(L, consts);
	return 1;
}

int push_evbuffer(lua_State* L, struct evbuffer* evbuf)
{
	lua_evbuffer* luaevbuf = (lua_evbuffer*)lua_newuserdata(L, sizeof(lua_evbuffer));
	if (!luaevbuf) return 0;

	luaevbuf->evbuf = evbuf;
	luaevbuf->bfree = 0;
	luaL_getmetatable(L, LUA_EVBUFFER_META);
	lua_setmetatable(L, -2);
	return 1;
}


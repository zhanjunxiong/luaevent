
#include "luabufferevent.h"
#include "luabuffer.h"
#include "luautil.h"

#include <event2/bufferevent.h>
#include <event2/util.h>

#include <assert.h> // for assert
#include <string.h> // for memset.h

#define LUABUFFEREVENT_INPUT 1
#define LUABUFFEREVENT_OUTPUT 2

static int luabufferevent_new(lua_State* L)
{
	lua_event_base* leb = (lua_event_base*)(luaL_checkudata(L, 1, LUA_EVENTBASE_META));

	struct event_base* base = leb->event_base;
	assert(base);

	evutil_socket_t fd = get_evutil_socket_t(L, 2);
	int options = luaL_checknumber(L, 3);

	struct bufferevent* bufev = bufferevent_socket_new(base, fd, options);
	if (!bufev) return 0;

	struct evbuffer* output = bufferevent_get_output(bufev);
	if (!output) return 0;

	struct evbuffer* input = bufferevent_get_input(bufev);
	if (!input) return 0;

	lua_bufferevent* luabufev = (lua_bufferevent*)lua_newuserdata(L, sizeof(lua_bufferevent));
	if (!luabufev) return 0;

	luabufev->leb = leb;
	luabufev->bufferevent = bufev;
	luabufev->readcb = -1;
	luabufev->writecb = -1;
	luabufev->eventcb = -1;

	luaL_getmetatable(L, LUA_BUFFEREVENT_META);
	lua_setmetatable(L, -2);

	lua_newtable(L);
	push_evbuffer(L, input);
	lua_rawseti(L, -2, LUABUFFEREVENT_INPUT);

	push_evbuffer(L, output);
	lua_rawseti(L, -2, LUABUFFEREVENT_OUTPUT);

	lua_setfenv(L, -2);
	return 1;
}

static int luabufferevent_free(lua_State* L)
{
	lua_bufferevent* luabufferevent = (lua_bufferevent*)(luaL_checkudata(L, 1, LUA_BUFFEREVENT_META));

	if (luabufferevent->readcb != -1)
	{
		luaL_unref(L, LUA_REGISTRYINDEX, luabufferevent->readcb);
	}
	if (luabufferevent->eventcb != -1)
	{
		luaL_unref(L, LUA_REGISTRYINDEX, luabufferevent->eventcb);
	}
	if (luabufferevent->writecb != -1)
	{
		luaL_unref(L, LUA_REGISTRYINDEX, luabufferevent->writecb);
	}

	if (luabufferevent->bufferevent)
	{
		bufferevent_free(luabufferevent->bufferevent);
		luabufferevent->bufferevent = NULL;
	}
	return 0;
}

static void luabufferevent_readdata_cb(struct bufferevent* bev, void* ctx)
{
	lua_bufferevent* luabufferevent = (lua_bufferevent*)ctx;
	if (!luabufferevent) return;

	lua_event_base* leb = luabufferevent->leb;
	lua_State* L = leb->L;

	if (luabufferevent->readcb != -1)
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, luabufferevent->readcb);
		if (lua_isfunction(L, -1))
		{
			lua_pcall(L, 0, 0, 0);
		}
		else
		{
			fprintf(stderr, "some error, readcb is not function\n");
		}
	}
	else
	{
		fprintf(stderr, "some error or not hook readdata_cb, luabufferevent_readdata_cb\n");
	}
}

static void luabufferevent_writedata_cb(struct bufferevent* bev, void* ctx)
{
	lua_bufferevent* luabufferevent = (lua_bufferevent*)ctx;
	if (!luabufferevent) return;

	lua_event_base* leb = luabufferevent->leb;
	lua_State* L = leb->L;

	if (luabufferevent->writecb != -1)
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, luabufferevent->writecb);
		if (lua_isfunction(L, -1))
		{
			lua_pcall(L, 0, 0, 0);
		}
		else
		{
			fprintf(stderr, "some error, writedata_cb is not function\n");
		}
	}
	else
	{
		fprintf(stderr, "some error or not hook writedata_cb, luabufferevent_writedata_cb\n");
	}
}

static void luabufferevent_event_cb(struct bufferevent *bev, short what, void *ctx)
{
	lua_bufferevent* luabufferevent = (lua_bufferevent*)ctx;
	if (!luabufferevent) return;

	lua_event_base* leb = luabufferevent->leb;
	lua_State* L = leb->L;

	if (luabufferevent->eventcb != -1)
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, luabufferevent->eventcb);
		if (lua_isfunction(L, -1))
		{
			lua_pushinteger(L, what);
			lua_pcall(L, 1, 0, 0);
		}
		else
		{
			fprintf(stderr, "some error, event_cb is not function\n");
		}
	}
	else
	{
		fprintf(stderr, "some error or not hook event_cb, luabufferevent_event_cb\n");
	}
}

static int luabufferevent_setcb(lua_State* L)
{
	lua_bufferevent* luabufferevent = (lua_bufferevent*)(luaL_checkudata(L, 1, LUA_BUFFEREVENT_META));

	if (!lua_isnil(L, 2))
	{
		luaL_checktype(L, 2, LUA_TFUNCTION);
		lua_pushvalue(L, 2);
		luabufferevent->readcb = lua_ref(L, LUA_REGISTRYINDEX);
	}

	if (!lua_isnil(L, 3))
	{
		luaL_checktype(L, 3, LUA_TFUNCTION);
		lua_pushvalue(L, 3);
		luabufferevent->writecb = lua_ref(L, LUA_REGISTRYINDEX);
	}

	if (!lua_isnil(L, 4))
	{
		luaL_checktype(L, 4, LUA_TFUNCTION);
		lua_pushvalue(L, 4);
		luabufferevent->eventcb = lua_ref(L, LUA_REGISTRYINDEX);
	}

	bufferevent_setcb(luabufferevent->bufferevent,
			luabufferevent_readdata_cb,
			luabufferevent_writedata_cb,
			luabufferevent_event_cb,
			luabufferevent);
	return 0;
}

static int luabufferevent_write(lua_State* L)
{
	lua_bufferevent* luabufferevent = (lua_bufferevent*)(luaL_checkudata(L, 1, LUA_BUFFEREVENT_META));

	size_t size;
	const char* data = luaL_checklstring(L, 2, &size);
	int ret = bufferevent_write(luabufferevent->bufferevent, data, size);
	lua_pushinteger(L, ret);
	return 1;
}

#define MAX_BUFFERSIZE 65536
static int luabufferevent_read(lua_State* L)
{
	lua_bufferevent* luabufferevent = (lua_bufferevent*)(luaL_checkudata(L, 1, LUA_BUFFEREVENT_META));

	char data[MAX_BUFFERSIZE];
	size_t write_size = bufferevent_read(luabufferevent->bufferevent, data, MAX_BUFFERSIZE);
	lua_pushlstring(L, data, write_size);
	lua_pushinteger(L, write_size);
	return 2;
}

static int luabufferevent_enable(lua_State* L)
{
	lua_bufferevent* luabufferevent = (lua_bufferevent*)(luaL_checkudata(L, 1, LUA_BUFFEREVENT_META));

	short event = luaL_checknumber(L, 2);
	int ret = bufferevent_enable(luabufferevent->bufferevent, event);
	lua_pushinteger(L, ret);
	return 1;
}

static int luabufferevent_disable(lua_State* L)
{
	lua_bufferevent* luabufferevent = (lua_bufferevent*)(luaL_checkudata(L, 1, LUA_BUFFEREVENT_META));

	short event = luaL_checknumber(L, 2);
	int ret = bufferevent_disable(luabufferevent->bufferevent, event);
	lua_pushinteger(L, ret);
	return 1;
}

static int luabufferevent_socket_connect(lua_State* L)
{
	lua_bufferevent* luabufferevent = (lua_bufferevent*)(luaL_checkudata(L, 1, LUA_BUFFEREVENT_META));

	const char* str = luaL_checkstring(L, 2);
	struct sockaddr_storage addr;
	memset(&addr, 0, sizeof(addr));
	int socklen = sizeof(addr);
	int ret = evutil_parse_sockaddr_port(str, (struct sockaddr*)&addr, &socklen);
	if (ret != 0)
	{
		fprintf(stderr, "luabufferevent_socket_connect addr(%s) error!!!\n", str);
		return 0;
	}

	ret = bufferevent_socket_connect(luabufferevent->bufferevent, (struct sockaddr *)&addr, socklen);
	lua_pushinteger(L, ret);
	return 1;
}

static int luabufferevent_setwatermark(lua_State* L)
{
	lua_bufferevent* luabufferevent = (lua_bufferevent*)(luaL_checkudata(L, 1, LUA_BUFFEREVENT_META));

	short events = luaL_checknumber(L, 2);
	size_t lowmark = luaL_checknumber(L, 3);
	size_t hightmark = luaL_checknumber(L, 4);
	bufferevent_setwatermark(luabufferevent->bufferevent, events, lowmark, hightmark);
	return 0;
}

static int luabufferevent_set_timeouts(lua_State* L)
{
	lua_bufferevent* luabufferevent = (lua_bufferevent*)(luaL_checkudata(L, 1, LUA_BUFFEREVENT_META));

	int read_sec = luaL_checknumber(L, 2);
	int read_usec = luaL_checknumber(L, 3);
	int write_sec = luaL_checknumber(L, 4);
	int write_usec = luaL_checknumber(L, 5);
	struct timeval timeout_read;
	timeout_read.tv_sec = read_sec;
	timeout_read.tv_usec = read_usec;

	struct timeval timeout_write;
	timeout_write.tv_sec = write_sec;
	timeout_write.tv_usec = write_usec;

	int ret = bufferevent_set_timeouts(luabufferevent->bufferevent, &timeout_read, &timeout_write);
	lua_pushinteger(L, ret);
	return 1;
}


static int luabufferevent_get_input(lua_State* L)
{
	(luaL_checkudata(L, 1, LUA_BUFFEREVENT_META));
	lua_getfenv(L, 1);
	lua_rawgeti(L, -1, LUABUFFEREVENT_INPUT);
	return 1;
}

static int luabufferevent_get_output(lua_State* L)
{
	(luaL_checkudata(L, 1, LUA_BUFFEREVENT_META));
	lua_getfenv(L, 1);
	lua_rawgeti(L, -1, LUABUFFEREVENT_OUTPUT);
	return 1;
}

static const luaL_Reg lib[] =
{
		{"new", luabufferevent_new},
		{NULL, NULL}
};

static const luaL_Reg libm[] =
{
		{"__gc", luabufferevent_free},
		{NULL, NULL}
};


static const luaL_Reg indexlib[] =
{
		{"setcb", luabufferevent_setcb},
		{"write", luabufferevent_write},
		{"read", luabufferevent_read},
		{"enable", luabufferevent_enable},
		{"disable", luabufferevent_disable},
		{"connect", luabufferevent_socket_connect},
		{"setwatermark", luabufferevent_setwatermark},
		{"set_timeouts", luabufferevent_set_timeouts},
		{"get_input", luabufferevent_get_input},
		{"get_output", luabufferevent_get_output},
		{"free", luabufferevent_free},
		{NULL, NULL}
};

static const name_integer consts[] =
{
		{"BEV_OPT_CLOSE_ON_FREE", BEV_OPT_CLOSE_ON_FREE},
		{"BEV_OPT_THREADSAFE", BEV_OPT_THREADSAFE},
		{"BEV_OPT_DEFER_CALLBACKS", BEV_OPT_DEFER_CALLBACKS},
		{"BEV_OPT_UNLOCK_CALLBACKS", BEV_OPT_UNLOCK_CALLBACKS},

		{"BEV_EVENT_READING", BEV_EVENT_READING},
		{"BEV_EVENT_WRITING", BEV_EVENT_WRITING},
		{"BEV_EVENT_EOF", BEV_EVENT_EOF},
		{"BEV_EVENT_ERROR", BEV_EVENT_ERROR},
		{"BEV_EVENT_TIMEOUT", BEV_EVENT_TIMEOUT},
		{"BEV_EVENT_CONNECTED", BEV_EVENT_CONNECTED},
		{NULL, 0}
};

LUALIB_API int luaopen_bufferevent(lua_State* L)
{
	luaL_newmetatable(L, LUA_BUFFEREVENT_META);
	register_index_fun(L, indexlib);
	luaL_register(L, NULL, libm);

	luaL_register(L, LUA_BUFFEREVENT, lib);
	set_name_integers(L, consts);
	return 1;
}


#include "luautil.h"

void set_name_integers(lua_State* L, const name_integer* p)
{
	while(p->name)
	{
		lua_pushnumber(L, p->value);
		lua_setfield(L, -2, p->name);
		p++;
	}
}

void register_index_fun(lua_State* L, const luaL_Reg* lib)
{
	lua_newtable(L);
	luaL_register(L, NULL, lib);
	lua_setfield(L, -2, "__index");
}

evutil_socket_t get_evutil_socket_t(lua_State* L, int stack_index)
{
	evutil_socket_t fd;
	if (lua_isuserdata(L, stack_index))
	{
		fd = (evutil_socket_t)lua_touserdata(L, stack_index);
	}
	else if (lua_isnumber(L, stack_index))
	{
		fd = lua_tonumber(L, stack_index);
	}
	else
	{
		luaL_typerror(L, stack_index, "userdata or number");
		return 0;
	}
	return fd;
}

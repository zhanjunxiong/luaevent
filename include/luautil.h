
#ifndef LUAUTIL_H_
#define LUAUTIL_H_

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <event2/util.h>

typedef struct tagname_integer
{
	const char* name;
	int value;
}name_integer;

void set_name_integers(lua_State* L, const name_integer* p);
void register_index_fun(lua_State* L, const luaL_Reg* lib);
evutil_socket_t get_evutil_socket_t(lua_State* L, int stack_index);

#endif

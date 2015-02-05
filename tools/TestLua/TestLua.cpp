#include <iostream>
#include <assert.h>

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

using namespace std;

//each of these takes a VEE object as an argument

int vee_putreg(lua_State *l) {
    //get offset, width, and value
    int offset;
    int width;
    int value;
    void *v;

    cout << "putreg" << endl;

    offset = luaL_checkinteger(l, 2);
    width = luaL_checkinteger(l, 3);
    value = luaL_checkinteger(l, 4);
    v = lua_touserdata(l, 1);

    return 0;
}

//either return a number or Nil
int vee_getreg(lua_State *l) {
    unsigned int offset;
    unsigned int width;
    void *v;
    //get offset and width
    offset = luaL_checkinteger(l, 2);
    width = luaL_checkinteger(l, 3);
    v = lua_touserdata(l, 1);

    cout << "width: " << hex << width << dec << endl;
    cout << "offset: " << hex << offset << dec << endl;

    lua_pushinteger(l, 0);
    return 1;
}

int vee_putmem(lua_State *l) {
    //get offset, width, and value
    int offset;
    int width;
    int value;
    void *v;

    offset = luaL_checkinteger(l, 2);
    width = luaL_checkinteger(l, 3);
    value = luaL_checkinteger(l, 4);
    v = lua_touserdata(l, 1);

    return 0;
}

//either return a number or Nil
int vee_getmem(lua_State *l) {
    //get offset and width
    int offset;
    int width;
    void *v;
    //get offset and width

    offset = luaL_checkinteger(l, 2);
    width = luaL_checkinteger(l, 3);
    v = lua_touserdata(l, 1);

    lua_pushinteger(l, 0);
    return 1;
}

int precallbackFun;
int postcallbackFun;

int vee_register(lua_State *l) {
    postcallbackFun = luaL_ref(l, LUA_REGISTRYINDEX);
    precallbackFun = luaL_ref(l, LUA_REGISTRYINDEX);
    return 0;
}

int vee_foo(lua_State *l) {
    cout << "foo" << endl;
    lua_getfield(l, 1, "CLASS");
    int j = lua_tointeger(l, -1);
    cout << j << endl;
    lua_pop(l, 1);
    lua_getfield(l, 1, "WIDTH");
    j = lua_tointeger(l, -1);
    cout << j << endl;

    return 0;
}

static const luaL_Reg vee_meths[] = {
    {"getmem", vee_getmem},
    {"putmem", vee_putmem},
    {"getreg", vee_getreg},
    {"putreg", vee_putreg},
    {"register", vee_register},
    {"foo", vee_foo},
    {0,0}
};

void reg(lua_State *l) {
    lua_newtable(l);
    luaL_setfuncs(l, vee_meths, 0);
    lua_pushvalue(l, -1);
    lua_setglobal(l, "vee");
    return;
}

int main(int argc, char *argv[]) {
    lua_State *l = luaL_newstate();
    luaL_openlibs(l);

    //register my lib
    reg(l);

    int r;
    //load the test file
    r = luaL_dofile(l, argv[1]);

    //call the functions
    /*int k = 88;
    lua_getfield(l, LUA_GLOBALSINDEX, "onPre");
    lua_pushlightuserdata(l, &k);
    lua_call(l, 1, 0);*/
    /*int k =9;
    lua_rawgeti(l, LUA_REGISTRYINDEX, precallbackFun);
    lua_pushlightuserdata(l, &k);
    lua_call(l, 1, 0);*/

    return 0; 
}

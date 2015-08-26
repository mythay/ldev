#include <string.h>
#include <stdio.h>
#include "lua.h"
#include "lauxlib.h"
#include "ldev_serial.h"
#include <unistd.h>


#define LUA_DEVHANDLE          "LDEV*"

typedef struct{
    int fd;
    char * type;
}LDEV_HANDLE_t;

static int getfield (lua_State *L, const char *key, int d) {
  int res;
  lua_getfield(L, -1, key);
  if (lua_isnumber(L, -1))
    res = (int)lua_tointeger(L, -1);
  else {
    if (d < 0)
      return luaL_error(L, "field " LUA_QS " missing in date table", key);
    res = d;
  }
  lua_pop(L, 1);
  return res;
}

static LDEV_HANDLE_t *todev (lua_State *L) {
  LDEV_HANDLE_t *p = (LDEV_HANDLE_t *)luaL_checkudata(L, 1, LUA_DEVHANDLE);
  return p;
}

static int tofd (lua_State *L) {
  LDEV_HANDLE_t *p = todev(L);
  if (p->fd == 0)
    luaL_error(L, "attempt to use a closed fd");
  return p->fd;
}


static int ldev_serial (lua_State *L) {
    LDEV_HANDLE_t *p = NULL;
    SERIALPORT_FD_OPT opt={0};
    const char* fpath = NULL;
    
    luaL_checktype(L, 1, LUA_TSTRING); // first argument must be a serial port path
    luaL_checktype(L, 2, LUA_TTABLE); // second argument must be a table contains the serial  port configuration
    lua_settop(L, 2);

    fpath = lua_tostring(L,1);

    opt.baudRate = getfield(L,"baud",115200);
    opt.dataBits= getfield(L,"bits",8);
    opt.stopBits= getfield(L,"stop",1);
    
    lua_getfield(L, -1, "parity");
    if(lua_isstring(L,-1))
    {
        const char* parity_s = lua_tostring(L,-1);
        switch(*parity_s)
        {
            case 'e':
            case 'E':
                opt.parity = SERIALPORT_PARITY_EVEN;
                break;
            case 'o':
            case 'O':
                opt.parity = SERIALPORT_PARITY_ODD;
                break;
            case 'n':
            case 'N':
                opt.parity = SERIALPORT_PARITY_NONE;
                break;
            default:
                return luaL_error(L, "invalid parity string " LUA_QS " ", parity_s);
        }
    }else
    {
        opt.parity = SERIALPORT_PARITY_NONE;
    }
    lua_pop(L, 1);

    p = (LDEV_HANDLE_t *)lua_newuserdata(L, sizeof(LDEV_HANDLE_t));
    memset(p, 0, sizeof(LDEV_HANDLE_t));
    luaL_getmetatable(L, LUA_DEVHANDLE);
    lua_setmetatable(L, -2);


    p->fd = ldev_serial_open(fpath, &opt);
    if(p->fd <= 0)
    {   
        lua_settop(L,0);
        lua_pushnil(L);
        lua_pushstring(L, "open serial fail.");
        return 2;
    }
    p->type = "serial";
    lua_replace(L,1);
    lua_settop(L,1);
    return 1;
}

static int ldev_close(lua_State *L) {
    
    LDEV_HANDLE_t *p = todev(L);
    if (p->fd <= 0)
    {
        luaL_error(L, "attempt to use a closed fd");
    }
    close(p->fd);  /* make sure argument is an open stream */
    p->fd = 0;
    lua_pushboolean(L, 1);
    return 1;
}

static int ldev_fd(lua_State *L) {
    lua_pushinteger(L, tofd(L));
    return 1;
}

static int ldev_gc(lua_State *L) {
    LDEV_HANDLE_t *p = todev(L);
    if (p->fd > 0)
    {
        close(p->fd);
    }
    return 0;
}

static int ldev_tostring (lua_State *L) {
  LDEV_HANDLE_t *p = todev(L);
  if (p->fd <= 0)
    lua_pushliteral(L, "ldev (released)");
  else
    lua_pushfstring(L, "ldev fd (%d)", p->fd);
  return 1;
}


static const luaL_Reg ldevlib[] = {
    {"serial", ldev_serial},
    {NULL, NULL}
};

/*
** methods for file handles
*/
static const luaL_Reg ldevlib_meta[] = {
    {"close", ldev_close},
    {"fileno", ldev_fd},
    {"__gc", ldev_gc},
    {"__tostring", ldev_tostring},
    {NULL, NULL}
};

static void create_devmeta (lua_State *L) {
  luaL_newmetatable(L, LUA_DEVHANDLE);  /* create metatable for file handles */
  lua_pushvalue(L, -1);  /* push metatable */
  lua_setfield(L, -2, "__index");  /* metatable.__index = metatable */
  luaL_register(L, NULL, ldevlib_meta);  /* file methods */
  lua_pop(L, 1);  /* pop new metatable */
}



LUALIB_API int luaopen_ldev (lua_State *L) {
  luaL_register(L, "ldev", ldevlib);
  create_devmeta(L);
  return 1;
}


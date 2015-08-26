cmake_minimum_required(VERSION 2.8)

project (ldev)

set(LDEV_VERSION_MAJOR 0)
set(LDEV_VERSION_MINOR 0)
set(LDEV_VERSION_PATCH 1)
set(LDEV_VERSION ${LDEV_VERSION_MAJOR}.${LDEV_VERSION_MINOR}.${LDEV_VERSION_PATCH})

find_package(Lua51 REQUIRED)
include_directories(${LUA_INCLUDE_DIR})
get_filename_component(_lua_lib_dir ${LUA_LIBRARY} PATH)
if(WIN32)
    # Win32 modules need to be linked to the Lua library.
    set(_MODULE_LINK ${LUA_LIBRARY} ${_MODULE_LINK})
    set(_lua_module_dir "${_lua_lib_dir}")
    # Windows sprintf()/strtod() handle NaN/inf differently. Not supported.
    add_definitions(-DDISABLE_INVALID_NUMBERS)
else()
    set(_lua_module_dir "${_lua_lib_dir}/lua/5.1")
endif()

set (LDEV_SRCS src/ldev.c src/ldev_serial.c)
add_definitions(-DLUA_USE_LINUX)

add_library (ldev SHARED ${LDEV_SRCS})
#target_link_libraries(ldev zg)
set_target_properties(ldev PROPERTIES PREFIX "")

install(TARGETS ldev
LIBRARY DESTINATION ${_lua_module_dir}
)
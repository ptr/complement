# -*- makefile -*- Time-stamp: <09/12/17 16:25:50 ptr>

# LUA_SRC = ${HOME}/labs/lua
LIBNAME = lua
MAJOR = 5
MINOR = 1
PATCH = 4
SRC_C = ${LUA_SRC}/src/lapi.c \
        ${LUA_SRC}/src/lcode.c \
        ${LUA_SRC}/src/ldebug.c \
        ${LUA_SRC}/src/ldo.c \
        ${LUA_SRC}/src/ldump.c \
        ${LUA_SRC}/src/lfunc.c \
        ${LUA_SRC}/src/lgc.c \
        ${LUA_SRC}/src/llex.c \
        ${LUA_SRC}/src/lmem.c \
        ${LUA_SRC}/src/lobject.c \
        ${LUA_SRC}/src/lopcodes.c \
        ${LUA_SRC}/src/lparser.c \
        ${LUA_SRC}/src/lstate.c \
        ${LUA_SRC}/src/lstring.c \
        ${LUA_SRC}/src/ltable.c \
        ${LUA_SRC}/src/ltm.c \
        ${LUA_SRC}/src/lundump.c \
        ${LUA_SRC}/src/lvm.c \
        ${LUA_SRC}/src/lzio.c \
        ${LUA_SRC}/src/lauxlib.c \
        ${LUA_SRC}/src/lbaselib.c \
        ${LUA_SRC}/src/ldblib.c \
        ${LUA_SRC}/src/liolib.c \
        ${LUA_SRC}/src/lmathlib.c \
        ${LUA_SRC}/src/loslib.c \
        ${LUA_SRC}/src/ltablib.c \
        ${LUA_SRC}/src/lstrlib.c \
        ${LUA_SRC}/src/loadlib.c \
        ${LUA_SRC}/src/linit.c

# -*- makefile -*- Time-stamp: <09/03/18 10:02:17 ptr>

SRCROOT := ../../../explore
COMPILER_NAME := gcc
_STATIC_BUILD := 1

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

INCLUDES += -I${LUA_SRC}
# DEFS += -DLUA_USE_LINUX
DEFS += -DLUA_USE_POSIX -DLUA_USE_DLOPEN

LDLIBS += -ldl -lm

# -*- makefile -*- Time-stamp: <09/03/19 14:48:39 yeti>

SRCROOT := ../../../explore
COMPILER_NAME := gcc

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

INCLUDES += -I${LUA_SRC}
DEFS += -DLUA_USE_LINUX
# DEFS += -DLUA_USE_POSIX -DLUA_USE_DLOPEN

release-shared:	LDFLAGS += -L../liblua/${OUTPUT_DIR}

dbg-shared:	LDFLAGS += -L../liblua/${OUTPUT_DIR_DBG}

LDLIBS += -ldl -lreadline -lhistory -lm

release-shared:	LDLIBS += -llua

dbg-shared:	LDLIBS += -lluag

# -*- makefile -*- Time-stamp: <09/03/19 09:31:43 ptr>

SRCROOT := ../../../../explore
COMPILER_NAME := gcc

# EXTRA_PRE = configure
# EXTRA_PRE_DBG = configure
# EXTRA_PRE_STLDBG = configure

BERKELEYDB_SRC ?= ${HOME}/labs/BerkeleyDB

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

INCLUDES += -I../build_unix -I${BERKELEYDB_SRC}
DEFS += -D_GNU_SOURCE -D_REENTRANT

release-shared:	LDFLAGS += -L../db/${OUTPUT_DIR}

dbg-shared:	LDFLAGS += -L../db/${OUTPUT_DIR_DBG}

ifndef WITHOUT_STLPORT
stldbg-shared:	LDFLAGS += -L../db/${OUTPUT_DIR_DBG}
endif

release-shared:	LDLIBS += -ldb

dbg-shared:	LDLIBS += -ldbg

ifndef WITHOUT_STLPORT
stldbg-shared:	LDLIBS += -ldbg
endif

# -*- Makefile -*- Time-stamp: <02/09/29 19:26:18 ptr>
# $Id$

BASEDIR := $(shell xtmp=`pwd`; xtmp=`dirname $$xtmp`; dirname $$xtmp)
LIBDIR  := $(shell xtmp=`pwd`; dirname $$xtmp)

COMPILER_NAME = gcc
include $(BASEDIR)/lib/net/Makefile.inc

all:	all-release all-debug

install:	install-dbg-shared install-stldbg-shared install-release-shared

all-shared:	release-shared dbg-shared stldbg-shared

all-debug:	dbg-shared stldbg-shared

all-release:	release-shared

include ${BASEDIR}/Makefiles/lib/Makefile.inc

INCLUDES += -I$(STLPORT_INCLUDE_DIR)

ifeq ($(OS_VER),Linux)
release-shared:	LDSEARCH = -L${STLPORT_LIB_DIR}
stldbg-shared:	LDSEARCH = -L${STLPORT_LIB_DIR}
dbg-shared:	LDSEARCH = -L${STLPORT_LIB_DIR}
endif

ifeq ($(OS_VER),SunOS)
release-shared : LDLIBS = -lstlport_gcc
stldbg-shared  : LDLIBS = -lstlport_gcc_stldebug
dbg-shared     : LDLIBS = -lstlport_gcc
else
release-shared : LDLIBS = -lstlport_gcc
stldbg-shared  : LDLIBS = -lstlport_gcc_stldebug
dbg-shared     : LDLIBS = -lstlport_gcc
endif


# -*- Makefile -*- Time-stamp: <03/07/07 12:00:18 ptr>
# $Id$

BASEDIR := $(shell xtmp=`pwd`; xtmp=`dirname $$xtmp`; dirname $$xtmp)
LIBDIR  := $(shell xtmp=`pwd`; dirname $$xtmp)

COMPILER_NAME = gcc
STLPORT_LIB_DIR = /export/home/ptr/STLport.lab/STLport/lib
STLPORT_INCLUDE_DIR = /export/home/ptr/STLport.lab/STLport/stlport
include $(BASEDIR)/lib/mt/Makefile.inc

all:	all-release all-debug

install:	install-dbg-shared install-stldbg-shared install-release-shared

all-shared:	release-shared dbg-shared stldbg-shared

all-debug:	dbg-shared stldbg-shared

all-release:	release-shared

include ${BASEDIR}/Makefiles/top.mak


INCLUDES += -I$(STLPORT_INCLUDE_DIR)

ifeq ($(OSNAME),linux)
release-shared:	LDSEARCH = -L${STLPORT_LIB_DIR}
stldbg-shared:	LDSEARCH = -L${STLPORT_LIB_DIR}
dbg-shared:	LDSEARCH = -L${STLPORT_LIB_DIR}
endif

ifeq ($(OSNAME),sunos)
release-shared : LDLIBS = -lstlport_gcc -lrt
stldbg-shared  : LDLIBS = -lstlport_gcc_stldebug -lrt
dbg-shared     : LDLIBS = -lstlport_gcc -lrt
else
release-shared : LDLIBS = -lstlport_gcc
stldbg-shared  : LDLIBS = -lstlport_gcc_stldebug
dbg-shared     : LDLIBS = -lstlport_gcc
endif


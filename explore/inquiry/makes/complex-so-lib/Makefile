# -*- Makefile -*- Time-stamp: <03/03/21 12:01:22 ptr>
# $Id$

BASEDIR := $(shell xtmp=`pwd`; xtmp=`dirname $$xtmp`; xtmp=`dirname $$xtmp`; dirname $$xtmp)
LIBDIR  := $(shell xtmp=`pwd`; dirname $$xtmp)

COMPILER_NAME = gcc
include $(BASEDIR)/inquiry/makes/complex-so-lib/Makefile.inc

echo:	
	echo ${BASEDIR}

all:	all-release all-debug

install:	install-dbg-shared install-stldbg-shared install-release-shared

all-shared:	release-shared dbg-shared stldbg-shared

all-debug:	dbg-shared stldbg-shared

all-release:	release-shared

include ${BASEDIR}/Makefiles/lib/Makefile.inc

INCLUDES += -I$(STLPORT_INCLUDE_DIR)

ifeq ($(OS_VER),Linux)
release-shared:	LDSEARCH = -L${STLPORT_LIB_DIR} -L${INSTALL_LIB_DIR}
stldbg-shared:	LDSEARCH = -L${STLPORT_LIB_DIR}
dbg-shared:	LDSEARCH = -L${STLPORT_LIB_DIR}
endif

ifeq ($(OS_VER),SunOS)
release-shared : LDLIBS = -lstlport_gcc -lrt
stldbg-shared  : LDLIBS = -lstlport_gcc_stldebug -lrt
dbg-shared     : LDLIBS = -lstlport_gcc -lrt
else
release-shared : LDLIBS = -lstlport_gcc
stldbg-shared  : LDLIBS = -lstlport_gcc_stldebug
dbg-shared     : LDLIBS = -lstlport_gcc
endif


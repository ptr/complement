# -*- makefile -*- Time-stamp: <02/12/02 14:10:15 ptr>
# $Id$

BASEDIR := $(shell xtmp=`pwd`; xtmp=`dirname $$xtmp`; xtmp=`dirname $$xtmp`; dirname $$xtmp)
COMPILER_NAME = gcc

include ./Makefile.inc

all:	release-shared

all-shared:	release-shared dbg-shared stldbg-shared

all-debug:	dbg-shared stldbg-shared

all-release:	release-shared

include ${BASEDIR}/Makefiles/app/Makefile.inc

ifeq ($(OS_VER),SunOS)
LDLIBS = 
else
LDLIBS = 
endif

LDLIBS += -lrt -lnsl

ifeq ($(OS_VER),SunOS)
LDLIBS += -lsocket
endif

FORCE:


# -*- makefile -*- Time-stamp: <02/12/02 12:49:02 ptr>
# $Id$

BASEDIR := $(shell xtmp=`pwd`; xtmp=`dirname $$xtmp`; xtmp=`dirname $$xtmp`; dirname $$xtmp)
COMPILER_NAME = gcc
include ./Makefile.inc

all:	release-shared

all-shared:	release-shared dbg-shared stldbg-shared

all-debug:	dbg-shared stldbg-shared

all-release:	release-shared

include ${BASEDIR}/Makefiles/app/Makefile.inc

INCLUDES +=
DEFS += 

ifeq ($(OS_VER),SunOS)
LDLIBS = -lrt
endif

FORCE:


# -*- Makefile -*- Time-stamp: <03/07/09 18:08:47 ptr>
# $Id$

SRCROOT := ../../..
COMPILER_NAME := gcc

#STLPORT_DIR := /export/home/ptr/STLport.lab/cvs.stlport.com/STLport-R451_dev
STLPORT_DIR := /export/home/ptr/STLport.lab/STLport
include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak


INCLUDES += -I$(SRCROOT)/include -I$(STLPORT_INCLUDE_DIR)

LDFLAGS += -Wl,-rpath=$(STLPORT_LIB_DIR)

release-shared:	LDSEARCH = -L${STLPORT_LIB_DIR}
stldbg-shared:	LDSEARCH = -L${STLPORT_LIB_DIR}
dbg-shared:	LDSEARCH = -L${STLPORT_LIB_DIR}

ifeq ($(OSNAME),sunos)
release-shared : LDLIBS = -lstlport_gcc -lrt
stldbg-shared  : LDLIBS = -lstlport_gcc_stldebug -lrt
dbg-shared     : LDLIBS = -lstlport_gcc -lrt
else
release-shared : LDLIBS = -Wl,-dn -lstlport_gcc -Wl,-dy
stldbg-shared  : LDLIBS = -Wl,-dn -lstlport_gcc_stldebug -Wl,-dy
dbg-shared     : LDLIBS = -Wl,-dn -lstlport_gcc -Wl,-dy
endif


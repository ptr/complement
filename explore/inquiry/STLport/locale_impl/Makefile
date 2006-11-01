# -*- Makefile -*- Time-stamp: <03/07/09 18:08:47 ptr>
# $Id$

SRCROOT := ../../..
COMPILER_NAME := gcc

STLPORT_DIR := /export/home/ptr/STLport.lab/cvs.stlport.com/STLport
include Makefile.inc
include ${SRCROOT}/Makefiles/top.mak


DEFS += -D_STLP_LEAKS_PEDANTIC
INCLUDES += -I$(SRCROOT)/include -I$(STLPORT_INCLUDE_DIR)

LDFLAGS += -Wl,-rpath=$(STLPORT_LIB_DIR)

release-shared:	LDSEARCH = -L${STLPORT_LIB_DIR}
stldbg-shared:	LDSEARCH = -L${STLPORT_LIB_DIR}
dbg-shared:	LDSEARCH = -L${STLPORT_LIB_DIR}

ifeq ($(OSNAME),sunos)
release-shared : LDLIBS = -lstlport -lrt
stldbg-shared  : LDLIBS = -lstlportstlg -lrt
dbg-shared     : LDLIBS = -lstlportg -lrt
else
release-shared : LDLIBS = -lstlport
stldbg-shared  : LDLIBS = -lstlportstlg
dbg-shared     : LDLIBS = -lstlportg
endif


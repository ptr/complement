# -*- Makefile -*- Time-stamp: <03/07/09 18:08:47 ptr>
# $Id$

SRCROOT := ../../../../..
COMPILER_NAME := gcc

ALL_TAGS := release-shared

STLPORT_DIR := /export/home/ptr/STLport.lab/cvs.stlport.com/STLport

include ../Makefile.inc
include ${SRCROOT}/Makefiles/top.mak


INCLUDES += -I$(STLPORT_INCLUDE_DIR) -I$(CoMT_INCLUDE_DIR)
DEFS += -D_STLP_USE_MALLOC

release-shared:	LDSEARCH = -L../../../build/lib/malloc -Wl,-rpath=../../../build/lib/malloc
# stldbg-shared:	LDSEARCH = -L${STLPORT_LIB_DIR} -L${CoMT_LIB_DIR_STLDBG}
# dbg-shared:	LDSEARCH = -L${STLPORT_LIB_DIR} -L${CoMT_LIB_DIR_DBG}

ifeq ($(OSNAME),sunos)
release-shared : LDLIBS = -lstlport_gcc -lmisc_gcc -lrt
# stldbg-shared  : LDLIBS = -lstlport_gcc_stldebug -lrt
# dbg-shared     : LDLIBS = -lstlport_gcc -lrt
else
release-shared : LDLIBS = -lstlport_gcc -lmisc_gcc
# stldbg-shared  : LDLIBS = -lstlport_gcc_stldebug -lxmt_gcc_stl-g
# dbg-shared     : LDLIBS = -lstlport_gcc -lxmt_gcc-g
endif


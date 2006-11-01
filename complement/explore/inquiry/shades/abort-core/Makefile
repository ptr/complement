# -*- Makefile -*- Time-stamp: <04/01/09 16:53:50 ptr>
# $Id$

SRCROOT := ../../..
COMPILER_NAME := gcc

include Makefile.inc
include ${SRCROOT}/Makefiles/top.mak

release-shared : LDFLAGS += -Wl,-rpath=${STLPORT_LIB_DIR}
dbg-shared : LDFLAGS += -Wl,-rpath=${STLPORT_LIB_DIR}
stldbg-shared : LDFLAGS += -Wl,-rpath=${STLPORT_LIB_DIR}

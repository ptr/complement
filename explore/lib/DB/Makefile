# -*- Makefile -*- Time-stamp: <05/10/03 22:50:40 ptr>
# $Id$

SRCROOT := ../..
COMPILER_NAME := gcc

include Makefile.inc
include ${SRCROOT}/Makefiles/top.mak

INCLUDES += -I$(SRCROOT)/include -I$(STLPORT_INCLUDE_DIR)

dbg-shared: DEFS += -D__DEBUG

LDLIBS += -ldl


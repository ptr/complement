# -*- Makefile -*- Time-stamp: <06/06/27 02:15:11 ptr>

SRCROOT := ../../../..
COMPILER_NAME := gcc

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak


INCLUDES += -I$(SRCROOT)/include -I$(BOOST_INCLUDE_DIR)
LDFLAGS += -Wl,--rpath=${STLPORT_LIB_DIR}

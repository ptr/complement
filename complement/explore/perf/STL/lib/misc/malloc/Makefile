# -*- Makefile -*- Time-stamp: <05/12/27 00:11:41 ptr>

SRCROOT := ../../../../..
COMPILER_NAME := gcc
INSTALL_LIB_DIR     := ../../../build/lib/malloc
ALL_TAGS := install-release-shared

include ../Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

INCLUDES += -I$(SRCROOT)/include
DEFS += -D_STLP_USE_MALLOC

ifeq ($(OSNAME),linux)
release-shared:	LDSEARCH = -L${INSTALL_LIB_DIR} -Wl,-rpath=${INSTALL_LIB_DIR}
stldbg-shared:	LDSEARCH = -L${STLPORT_LIB_DIR}
dbg-shared:	LDSEARCH = -L${STLPORT_LIB_DIR}
endif


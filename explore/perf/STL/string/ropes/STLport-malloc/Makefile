# -*- Makefile -*- Time-stamp: <06/01/03 18:35:21 ptr>

SRCROOT := ../../../../..
COMPILER_NAME := gcc

ALL_TAGS := release-shared


include ../Makefile.inc
include ${SRCROOT}/Makefiles/top.mak

INCLUDES += -I${CoMT_INCLUDE_DIR}
DEFS += -D_STLP_USE_MALLOC

release-shared:	LDSEARCH = -L../../../build/lib/malloc -Wl,-rpath=../../../build/lib/malloc
release-shared: LDLIBS = -lmisc
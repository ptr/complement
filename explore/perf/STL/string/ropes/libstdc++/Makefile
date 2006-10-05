# -*- Makefile -*- Time-stamp: <06/01/03 21:29:55 ptr>

SRCROOT := ../../../../..
COMPILER_NAME := gcc

ALL_TAGS := release-shared


WITHOUT_STLPORT := 1

include ../Makefile.inc
include ${SRCROOT}/Makefiles/top.mak

INCLUDES += -I${CoMT_INCLUDE_DIR}

release-shared:	LDSEARCH = -L../../../build/lib/libstdc++ -Wl,-rpath=../build/lib/libstdc++
release-shared: LDLIBS = -lmisc

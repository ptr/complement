# -*- Makefile -*- Time-stamp: <03/07/09 18:08:47 ptr>

SRCROOT := ../../../../..
COMPILER_NAME := gcc

ALL_TAGS := release-shared


include ../Makefile.inc
include ${SRCROOT}/Makefiles/top.mak


INCLUDES += -I$(STLPORT_INCLUDE_DIR)
DEFS += -D_STLP_USE_MALLOC

release-shared:	LDSEARCH = -L../../../build/lib/malloc -Wl,-rpath=../../../build/lib/malloc

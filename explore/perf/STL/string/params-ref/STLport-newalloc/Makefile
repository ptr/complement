# -*- Makefile -*- Time-stamp: <05/12/27 00:35:39 ptr>

SRCROOT := ../../../../..
COMPILER_NAME := gcc

ALL_TAGS := release-shared


include ../Makefile.inc
include ${SRCROOT}/Makefiles/top.mak

INCLUDES += -I$(STLPORT_INCLUDE_DIR)
DEFS += -D_STLP_USE_NEWALLOC

release-shared:	LDSEARCH = -L../../../build/lib/newalloc -Wl,-rpath=../build/lib/newalloc

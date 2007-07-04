# -*- Makefile -*- Time-stamp: <05/12/27 00:17:45 ptr>

SRCROOT := ../../../../..
COMPILER_NAME := gcc

ALL_TAGS := release-shared


include ../Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

DEFS += -D_STLP_USE_NEWALLOC

release-shared:	LDSEARCH = -L../../../build/lib/newalloc -Wl,-rpath=../build/lib/newalloc

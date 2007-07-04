# -*- Makefile -*- Time-stamp: <05/12/27 01:05:47 ptr>

SRCROOT := ../../../../..
COMPILER_NAME := gcc

ALL_TAGS := release-shared


include ../Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

DEFS += -D_STLP_USE_MALLOC -D_STLP_DONT_USE_SHORT_STRING_OPTIM

release-shared:	LDSEARCH = -L../../../build/lib/noshortstr-ma -Wl,-rpath=../build/lib/noshortstr-ma

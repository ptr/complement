# -*- Makefile -*- Time-stamp: <05/12/31 00:11:20 ptr>

SRCROOT := ../../../../..
COMPILER_NAME := gcc

ALL_TAGS := release-shared

STLPORT_LIB_DIR := ../../../build/lib/malloc

include ../Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

DEFS += -D_STLP_USE_MALLOC

release-shared:	LDSEARCH = -L../../../build/lib/malloc -Wl,-rpath=../build/lib/malloc


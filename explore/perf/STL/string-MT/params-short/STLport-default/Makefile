# -*- Makefile -*- Time-stamp: <05/12/27 01:05:22 ptr>

SRCROOT := ../../../../..
COMPILER_NAME := gcc

ALL_TAGS := release-shared


include ../Makefile.inc
include ${SRCROOT}/Makefiles/top.mak

release-shared:	LDSEARCH = -L../../../build/lib -Wl,-rpath=../../../build/lib

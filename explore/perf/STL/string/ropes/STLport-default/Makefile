# -*- Makefile -*- Time-stamp: <06/01/03 18:34:27 ptr>

SRCROOT := ../../../../..
COMPILER_NAME := gcc

ALL_TAGS := release-shared

STLPORT_DIR := /export/home/ptr/STLport.lab/STLport

include ../Makefile.inc
include ${SRCROOT}/Makefiles/top.mak

INCLUDES += -I${CoMT_INCLUDE_DIR}

release-shared:	LDSEARCH = -L../../../build/lib -Wl,-rpath=../../../build/lib
release-shared: LDLIBS = -lmisc
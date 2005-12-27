# -*- Makefile -*- Time-stamp: <05/12/27 00:30:33 ptr>

SRCROOT := ../../../../..
COMPILER_NAME := gcc

ALL_TAGS := release-shared

STLPORT_DIR := /export/home/ptr/STLport.lab/STLport

include ../Makefile.inc
include ${SRCROOT}/Makefiles/top.mak

DEFS += -D_STLP_USE_MALLOC

release-shared:	LDSEARCH = -L../../../build/lib/malloc -Wl,-rpath=../../../build/lib/malloc

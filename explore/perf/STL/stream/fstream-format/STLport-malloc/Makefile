# -*- Makefile -*- Time-stamp: <05/12/31 00:06:49 ptr>

SRCROOT := ../../../../..
COMPILER_NAME := gcc

ALL_TAGS := release-shared

STLPORT_DIR := /export/home/ptr/STLport.lab/STLport
STLPORT_LIB_DIR := ../../../build/lib/malloc

include ../Makefile.inc
include ${SRCROOT}/Makefiles/top.mak

DEFS += -D_STLP_USE_MALLOC

release-shared:	LDSEARCH = -L../../../build/lib/malloc -Wl,-rpath=../../../build/lib/malloc

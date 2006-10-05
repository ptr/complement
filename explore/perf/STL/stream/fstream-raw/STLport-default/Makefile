# -*- Makefile -*- Time-stamp: <05/12/31 00:08:25 ptr>

SRCROOT := ../../../../..
COMPILER_NAME := gcc

ALL_TAGS := release-shared

STLPORT_LIB_DIR := ../../../build/lib

include ../Makefile.inc
include ${SRCROOT}/Makefiles/top.mak


release-shared:	LDSEARCH = -L../../../build/lib -Wl,-rpath=../build/lib

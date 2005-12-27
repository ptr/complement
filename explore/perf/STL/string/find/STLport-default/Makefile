# -*- Makefile -*- Time-stamp: <05/12/27 00:25:37 ptr>

SRCROOT := ../../../../..
COMPILER_NAME := gcc

ALL_TAGS := release-shared

STLPORT_DIR := /export/home/ptr/STLport.lab/STLport

include ../Makefile.inc
include ${SRCROOT}/Makefiles/top.mak

release-shared:	LDSEARCH = -L../../../build/lib -Wl,-rpath=../../../build/lib

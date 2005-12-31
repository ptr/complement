# -*- Makefile -*- Time-stamp: <05/12/30 23:54:29 ptr>

SRCROOT := ../../../../..
COMPILER_NAME := gcc

ALL_TAGS := release-shared

STLPORT_DIR := /export/home/ptr/STLport.lab/STLport
STLPORT_LIB_DIR := ../../../build/lib

include ../Makefile.inc
include ${SRCROOT}/Makefiles/top.mak

release-shared:	LDSEARCH = -L../../../build/lib -Wl,-rpath=../../../build/lib


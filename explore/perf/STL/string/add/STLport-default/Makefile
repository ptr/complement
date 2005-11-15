# -*- Makefile -*- Time-stamp: <03/07/09 18:08:47 ptr>
# $Id$

SRCROOT := ../../../../..
COMPILER_NAME := gcc

ALL_TAGS := release-shared

STLPORT_DIR := /export/home/ptr/STLport.lab/STLport

include ../Makefile.inc
include ${SRCROOT}/Makefiles/top.mak


INCLUDES += -I$(STLPORT_INCLUDE_DIR)

release-shared:	LDSEARCH = -L../../../build/lib -Wl,-rpath=../../../build/lib


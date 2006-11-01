# -*- Makefile -*- Time-stamp: <05/12/27 00:41:26 ptr>

SRCROOT := ../../../../..
COMPILER_NAME := gcc

ALL_TAGS := release-shared


include ../Makefile.inc
include ${SRCROOT}/Makefiles/top.mak

DEFS += -D_STLP_USE_TEMPLATE_EXPRESSION

release-shared:	LDSEARCH = -L../../../build/lib/texpr -Wl,-rpath=../build/lib/texpr

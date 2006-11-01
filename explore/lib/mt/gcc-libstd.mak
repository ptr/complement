# -*- Makefile -*- Time-stamp: <03/09/25 12:02:31 ptr>
# $Id$

SRCROOT := ../..
COMPILER_NAME := gcc

ALL_TAGS = release-shared dbg-shared

include Makefile.inc
include ${SRCROOT}/Makefiles/top.mak

INCLUDES += -I$(SRCROOT)/include


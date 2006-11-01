# -*- Makefile -*- Time-stamp: <03/07/11 13:11:50 ptr>
# $Id$

SRCROOT := ../..
COMPILER_NAME := vc6

include Makefile.inc
include ${SRCROOT}/Makefiles/top.msk


INCLUDES := /I "$(SRCROOT)/include" /I "$(STLPORT_INCLUDE_DIR)"

LDSEARCH := /LIBPATH:$(STLPORT_LIB_DIR)
release-shared: LDLIBS += stlport_vc6.lib


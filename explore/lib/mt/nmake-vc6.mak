# -*- Makefile -*- Time-stamp: <03/09/28 19:14:05 ptr>
# $Id$

SRCROOT=..\..
COMPILER_NAME=vc6

!include Makefile.inc

INCLUDES=$(INCLUDES) /I "$(SRCROOT)/include" /I "$(STLPORT_INCLUDE_DIR)"
OPT_STLDBG = /Zm800
LDSEARCH=$(LDSEARCH) /LIBPATH:$(STLPORT_LIB_DIR)

!include $(SRCROOT)/Makefiles/nmake/top.mak

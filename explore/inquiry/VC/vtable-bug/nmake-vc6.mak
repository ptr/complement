# -*- Makefile -*- Time-stamp: <03/09/28 18:51:32 ptr>
# $Id$

SRCROOT=../../..
COMPILER_NAME=vc6

!include Makefile.inc

INCLUDES=$(INCLUDES) /I "$(SRCROOT)/include" /I "$(STLPORT_INCLUDE_DIR)"

LDSEARCH=$(LDSEARCH) /LIBPATH:$(STLPORT_LIB_DIR) /LIBPATH:$(CoMT_LIB_DIR_DBG)
LDLIBS=xmt_vc6d.lib

!include $(SRCROOT)/Makefiles/nmake/top.mak

# -*- Makefile -*- Time-stamp: <03/07/11 13:11:50 ptr>
# $Id$

SRCROOT=../..
COMPILER_NAME=vc6

!include Makefile.inc

INCLUDES=$(INCLUDES) /I "$(SRCROOT)/include" /I "$(STLPORT_INCLUDE_DIR)"

LDSEARCH=$(LDSEARCH) /LIBPATH:$(STLPORT_LIB_DIR)

!include $(SRCROOT)/Makefiles/nmake/top.mak


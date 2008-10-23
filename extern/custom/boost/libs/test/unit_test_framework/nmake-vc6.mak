# -*- Makefile -*- Time-stamp: <03/09/28 19:14:05 ptr>
# $Id$

SRCROOT=..\..\..\..\..\..\explore
COMPILER_NAME=vc6

!include Makefile.inc

INCLUDES=/I "$(STLPORT_INCLUDE_DIR)" /I "$(BOOST_INCLUDE_DIR)"

DEFS=/D_STLP_USE_DYNAMIC_LIB
LDSEARCH=/LIBPATH:$(STLPORT_LIB_DIR)

!include $(SRCROOT)/Makefiles/nmake/top.mak

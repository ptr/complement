# -*- Makefile -*- Time-stamp: <03/08/18 12:09:33 ptr>
# $Id$

SRCROOT=..\..\..
COMPILER_NAME=vc6

!include Makefile.inc

INCLUDES=/I "$(STLPORT_INCLUDE_DIR)"
DEFS=/D_STLP_USE_DYNAMIC_LIB
OPT_STLDBG=/FR

#LDSEARCH=/LIBPATH:"e:\STLport-R451_dev\lib" /LIBPATH:"$(CoMT_LIB_DIR)" /NODEFAULTLIB:msvcrt.lib
LDSEARCH=/LIBPATH:"$(STLPORT_DIR)/lib"
#LDLIBS = boost_test_utf_vc6.lib xmt_vc6.lib
!include $(SRCROOT)/Makefiles/nmake/top.mak

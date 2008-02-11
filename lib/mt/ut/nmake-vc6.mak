# -*- Makefile -*- Time-stamp: <03/10/17 19:42:29 ptr>
# $Id$

SRCROOT=..\..\..
COMPILER_NAME=vc6

!include Makefile.inc

INCLUDES=$(INCLUDES) /I "$(SRCROOT)/include" /I "$(STLPORT_INCLUDE_DIR)" /I "$(BOOST_INCLUDE_DIR)"
DEFS = $(DEFS) /D_STLP_USE_DYNAMIC_LIB

#LDSEARCH=/LIBPATH:"e:\STLport-R451_dev\lib" /LIBPATH:"$(CoMT_LIB_DIR)" /NODEFAULTLIB:msvcrt.lib
LDSEARCH=/LIBPATH:"$(CoMT_LIB_DIR)"
#LDLIBS = boost_test_utf_vc6.lib xmt_vc6.lib libcmt.lib kernel32.lib stlport_vc6.lib
LDLIBS = xmt_vc6.lib boost_test_utf_vc6s.lib
!include $(SRCROOT)/Makefiles/nmake/top.mak


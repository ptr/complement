# -*- Makefile -*- Time-stamp: <03/07/09 18:08:47 ptr>
# $Id$

SRCROOT := ../../..
COMPILER_NAME := vc6

include Makefile.inc
include ${SRCROOT}/Makefiles/top.mak

INCLUDES += -I$(SRCROOT)/include -I$(STLPORT_INCLUDE_DIR)

MS_LIB_OPT = /LIBPATH:"c:\Program Files\Microsoft Visual Studio\VC98\Lib" /NODEFAULTLIB:libcmt.lib
release-shared:	LDSEARCH = /LIBPATH:${STLPORT_LIB_DIR} /LIBPATH:${CoMT_LIB_DIR} $(MS_LIB_OPT)
stldbg-shared:	LDSEARCH = -L${STLPORT_LIB_DIR} -L${CoMT_LIB_DIR_STLDBG} $(MS_LIB_OPT)
dbg-shared:	LDSEARCH = -L${STLPORT_LIB_DIR} -L${CoMT_LIB_DIR_DBG} $(MS_LIB_OPT)

release-shared : LDLIBS = xmt_vc6.lib
stldbg-shared  : LDLIBS = xmt_vc6_stld.lib
dbg-shared     : LDLIBS = xmt_vc6d.lib


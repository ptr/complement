# -*- Makefile -*- Time-stamp: <03/07/09 18:08:47 ptr>
# $Id$

SRCROOT := ../../..
COMPILER_NAME := vc6

include Makefile.inc
include ${SRCROOT}/Makefiles/top.mak

INCLUDES += -I$(SRCROOT)/include -I$(STLPORT_INCLUDE_DIR)

release-shared:	LDSEARCH = /LIBPATH:${STLPORT_LIB_DIR} /LIBPATH:${CoMT_LIB_DIR} /LIBPATH:"c:\Program Files\Microsoft Visual Studio\VC98\Lib"
stldbg-shared:	LDSEARCH = -L${STLPORT_LIB_DIR} -L${CoMT_LIB_DIR_STLDBG}
dbg-shared:	LDSEARCH = -L${STLPORT_LIB_DIR} -L${CoMT_LIB_DIR_DBG}

release-shared : LDLIBS = stlport_vc6.lib xmt_vc6.lib
stldbg-shared  : LDLIBS = -lstlport_gcc_stldebug -lxmt_gcc_stl-g
dbg-shared     : LDLIBS = -lstlport_gcc -lxmt_gcc-g


# -*- Makefile -*- Time-stamp: <03/08/15 12:46:45 ptr>
# $Id$

SRCROOT := ../../../../../explore
COMPILER_NAME := gcc

include Makefile.inc
include ${SRCROOT}/Makefiles/top.mak


INCLUDES += -I$(SRCROOT)/include -I$(STLPORT_INCLUDE_DIR) -I$(BOOST_INCLUDE_DIR)

release-shared:	LDSEARCH = -L${STLPORT_LIB_DIR} -L${CoMT_LIB_DIR}
stldbg-shared:	LDSEARCH = -L${STLPORT_LIB_DIR} -L${CoMT_LIB_DIR_STLDBG}
dbg-shared:	LDSEARCH = -L${STLPORT_LIB_DIR} -L${CoMT_LIB_DIR_DBG}

release-shared : LDLIBS = -Wl,-rpath=${STLPORT_LIB_DIR}:${CoMT_LIB_DIR} -lstlport_gcc -lboost_fs_gcc
stldbg-shared  : LDLIBS = -lstlport_gcc_stldebug -lboost_fs_gcc_stl-g
dbg-shared     : LDLIBS = -lstlport_gcc -lboost_fs_gcc-g

# -*- Makefile -*- Time-stamp: <05/02/22 22:27:37 ptr>
# $Id$

SRCROOT := ../../../../../explore
COMPILER_NAME := gcc

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak


INCLUDES += -I$(SRCROOT)/include -I$(STLPORT_INCLUDE_DIR) -I$(BOOST_INCLUDE_DIR)

release-shared:	LDSEARCH = -L${STLPORT_LIB_DIR} -L${CoMT_LIB_DIR}
stldbg-shared:	LDSEARCH = -L${STLPORT_LIB_DIR} -L${CoMT_LIB_DIR_STLDBG}
dbg-shared:	LDSEARCH = -L${STLPORT_LIB_DIR} -L${CoMT_LIB_DIR_DBG}

release-shared : LDLIBS = -Wl,-rpath=${STLPORT_LIB_DIR}:${CoMT_LIB_DIR} -lstlport -lboost_fs
stldbg-shared  : LDLIBS = -lstlportstlg -lboost_fsstlg
dbg-shared     : LDLIBS = -lstlport -lboost_fs

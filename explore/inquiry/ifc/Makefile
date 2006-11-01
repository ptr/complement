# -*- Makefile -*- Time-stamp: <04/06/04 16:55:19 ptr>

SRCROOT := ../..
COMPILER_NAME := gcc

include Makefile.inc
include ${SRCROOT}/Makefiles/top.mak

release-shared:	LDSEARCH = -L${STLPORT_LIB_DIR} -L${CoMT_LIB_DIR}
stldbg-shared:	LDSEARCH = -L${STLPORT_LIB_DIR} -L${CoMT_LIB_DIR_STLDBG}
dbg-shared:	LDSEARCH = -L${STLPORT_LIB_DIR} -L${CoMT_LIB_DIR_DBG}

release-shared : LDLIBS = -Wl,-rpath=${STLPORT_LIB_DIR}:${CoMT_LIB_DIR}
stldbg-shared  : LDLIBS = 
dbg-shared     : LDLIBS = 

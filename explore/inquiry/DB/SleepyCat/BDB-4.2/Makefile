# -*- Makefile -*- Time-stamp: <04/08/11 15:35:29 ptr>
# $Id$

SRCROOT := ../../../..
COMPILER_NAME := gcc
BDB_DIR := /opt/BDB-4.2

include Makefile.inc
include ${SRCROOT}/Makefiles/top.mak


INCLUDES += -I$(SRCROOT)/include -I$(STLPORT_INCLUDE_DIR) -I$(BOOST_INCLUDE_DIR) -I${BDB_DIR}/include

release-shared:	LDSEARCH = -L${STLPORT_LIB_DIR} -L${CoMT_LIB_DIR}
stldbg-shared:	LDSEARCH = -L${STLPORT_LIB_DIR} -L${CoMT_LIB_DIR_STLDBG}
dbg-shared:	LDSEARCH = -L${STLPORT_LIB_DIR} -L${CoMT_LIB_DIR_DBG} -L${BDB_DIR}/lib

release-shared : LDLIBS = -Wl,-rpath=${STLPORT_LIB_DIR}:${CoMT_LIB_DIR} -lstlport_gcc
stldbg-shared  : LDLIBS = -lstlport_gcc_stldebug -lboost_fs_gcc_stl-g
dbg-shared     : LDLIBS =  -Wl,-rpath=${STLPORT_LIB_DIR}:${CoMT_LIB_DIR}:${BDB_DIR}/lib -lstlport_gcc -lboost_fs_gcc-g -ldb

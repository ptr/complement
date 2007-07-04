# -*- Makefile -*- Time-stamp: <06/09/13 09:52:37 ptr>

SRCROOT := ../../../..
COMPILER_NAME := gcc

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

INCLUDES += -I${CoMT_INCLUDE_DIR}
LDSEARCH += -L${CoMT_LIB_DIR}

release-shared : LDFLAGS += -Wl,-rpath=${PWD}/${STLPORT_LIB_DIR}:${PWD}/${CoMT_LIB_DIR} -lxmt -lsockios -lstem
#dbg-shared : LDFLAGS += -Wl,-rpath=${STLPORT_LIB_DIR}:${PWD}/${CoMT_LIB_DIR} -lxmtg -lsockiosg -lstemg
#stldbg-shared : LDFLAGS += -Wl,-rpath=${STLPORT_LIB_DIR}:${PWD}/${CoMT_LIB_DIR} -lxmtstlg -lsockiosstlg -lstemstlg

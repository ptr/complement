# -*- Makefile -*- Time-stamp: <06/08/04 12:09:33 ptr>

SRCROOT := ../../..
COMPILER_NAME := gcc

include Makefile.inc
include ${SRCROOT}/Makefiles/top.mak


INCLUDES += -I$(SRCROOT)/include -I$(BOOST_INCLUDE_DIR)

LDSEARCH = -L${STLPORT_LIB_DIR} -L${CoMT_LIB_DIR}

release-shared : LDLIBS = -lxmt -lsockios -lstem -lboost_test_utf -ldl
stldbg-shared  : LDLIBS = -lxmtstlg -lsockiosstlg -lstemstlg -lboost_test_utfstlg -ldl
dbg-shared     : LDLIBS = -lxmtg -lsockiosg -lstemg -lboost_test_utfg -ldl

dbg-shared:	DEFS += -DDEBUG

LDFLAGS += -Wl,-rpath=${STLPORT_LIB_DIR}:${CoMT_LIB_DIR}

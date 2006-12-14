# -*- Makefile -*- Time-stamp: <03/08/15 12:46:45 ptr>

SRCROOT := ../..
COMPILER_NAME := gcc

include Makefile.inc
include ${SRCROOT}/Makefiles/top.mak


INCLUDES += -I$(SRCROOT)/include -I$(BOOST_INCLUDE_DIR)

LDSEARCH += -L${CoMT_LIB_DIR}

release-shared : LDLIBS = -lxmt -lsockios -lboost_test_utf
stldbg-shared  : LDLIBS = -lxmtstlg -lsockiosstlg -lboost_test_utfstlg
dbg-shared     : LDLIBS = -lxmtg -lsockiosg -lboost_test_utfg

LDFLAGS += -Wl,-rpath=${STLPORT_LIB_DIR}:${CoMT_LIB_DIR}

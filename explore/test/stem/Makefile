# -*- Makefile -*- Time-stamp: <06/12/13 01:07:22 ptr>

SRCROOT := ../..
COMPILER_NAME := gcc

EXTRA_POST := dl
EXTRA_POST_DBG := dl-dbg
EXTRA_POST_STLDBG := dl-stldbg

include Makefile.inc
include ${SRCROOT}/Makefiles/top.mak


INCLUDES += -I$(SRCROOT)/include -I$(BOOST_INCLUDE_DIR)

LDSEARCH = -L${STLPORT_LIB_DIR} -L${CoMT_LIB_DIR}

release-shared : LDLIBS = -lxmt -lsockios -lstem -lboost_test_utf -ldl
stldbg-shared  : LDLIBS = -lxmtstlg -lsockiosstlg -lstemstlg -lboost_test_utfstlg -ldl
dbg-shared     : LDLIBS = -lxmtg -lsockiosg -lstemg -lboost_test_utfg -ldl

dbg-shared:	DEFS += -DDEBUG

LDFLAGS += -Wl,-rpath=${STLPORT_LIB_DIR}:${CoMT_LIB_DIR}

PHONY += dl dl-dbg dl-stldbg

dl:
	${MAKE} -C dl release-shared

dl-dbg:
	${MAKE} -C dl dbg-shared

dl-stldbg:
	${MAKE} -C dl stldbg-shared

# -*- Makefile -*- Time-stamp: <03/07/09 18:08:47 ptr>

SRCROOT := ../../..
COMPILER_NAME := gcc

include Makefile.inc
include ${SRCROOT}/Makefiles/top.mak

INCLUDES += -I$(SRCROOT)/include -I$(BOOST_INCLUDE_DIR)

LDFLAGS += -Wl,-rpath=$(STLPORT_LIB_DIR):${CoMT_LIB_DIR}

release-shared : LDLIBS = -L${CoMT_LIB_DIR} -lboost_test_utf
stldbg-shared  : LDLIBS = -L${CoMT_LIB_DIR} -lboost_test_utfstlg
dbg-shared     : LDLIBS = -L${CoMT_LIB_DIR} -lboost_test_utfg

# -*- Makefile -*- Time-stamp: <06/06/07 09:15:35 ptr>

SRCROOT := ../../../..
BOOST_DIR := ${HOME}/labs/boost
COMPILER_NAME := gcc

include Makefile.inc
include ${SRCROOT}/Makefiles/top.mak


INCLUDES += -I$(SRCROOT)/include -I$(BOOST_INCLUDE_DIR)

# temporary, before dums fix strings:
# DEFS += -D_STLP_DONT_USE_TEMPLATE_EXPRESSION

ifeq ($(OSNAME),linux)
release-shared:	LDSEARCH += -L${CoMT_LIB_DIR} -Wl,--rpath=${CoMT_LIB_DIR}:${STLPORT_LIB_DIR}
stldbg-shared:	LDSEARCH += -L${CoMT_LIB_DIR_STLDBG} -Wl,--rpath=${CoMT_LIB_DIR_STLDBG}:${STLPORT_LIB_DIR}
dbg-shared:	LDSEARCH += -L${CoMT_LIB_DIR_DBG} -Wl,--rpath=${CoMT_LIB_DIR_DBG}:${STLPORT_LIB_DIR}
endif

ifeq ($(OSNAME),openbsd)
release-shared:	LDSEARCH += -L${CoMT_LIB_DIR} -Wl,-R${CoMT_LIB_DIR}:${STLPORT_LIB_DIR}
stldbg-shared:	LDSEARCH += -L${CoMT_LIB_DIR_STLDBG} -Wl,-R${CoMT_LIB_DIR_STLDBG}:${STLPORT_LIB_DIR}
dbg-shared:	LDSEARCH += -L${CoMT_LIB_DIR_DBG} -Wl,-R${CoMT_LIB_DIR_DBG}:${STLPORT_LIB_DIR}
endif

release-shared : LDLIBS = -lDB -lxmt -lboost_test_utf
stldbg-shared  : LDLIBS = -lDB -lboost_test_utfstlg
dbg-shared     : LDLIBS = -lDB -lboost_test_utfg

ifeq ($(OSNAME),freebsd)
release-shared : LDLIBS += -lthr
stldbg-shared  : LDLIBS += -lthr
dbg-shared     : LDLIBS += -lthr
endif

ifeq ($(OSNAME),sunos)
release-shared : LDLIBS += -lrt
stldbg-shared  : LDLIBS += -lrt
dbg-shared     : LDLIBS += -lrt
endif



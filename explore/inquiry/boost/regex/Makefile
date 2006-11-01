# -*- Makefile -*- Time-stamp: <03/11/21 08:02:55 ptr>
# $Id$

SRCROOT := ../../..
COMPILER_NAME := gcc

include Makefile.inc
include ${SRCROOT}/Makefiles/top.mak


INCLUDES += -I$(SRCROOT)/include -I$(STLPORT_INCLUDE_DIR) -I$(BOOST_INCLUDE_DIR)

# temporary, before dums fix strings:
# DEFS += -D_STLP_DONT_USE_TEMPLATE_EXPRESSION

ifeq ($(OSNAME),linux)
release-shared:	LDSEARCH = -L${STLPORT_LIB_DIR} -L${CoMT_LIB_DIR} -Wl,--rpath=${CoMT_LIB_DIR}:${STLPORT_LIB_DIR}
stldbg-shared:	LDSEARCH = -L${STLPORT_LIB_DIR} -L${CoMT_LIB_DIR_STLDBG} -Wl,--rpath=${CoMT_LIB_DIR_STLDBG}
dbg-shared:	LDSEARCH = -L${STLPORT_LIB_DIR} -L${CoMT_LIB_DIR_DBG} -Wl,--rpath=${CoMT_LIB_DIR_DBG}
endif

ifeq ($(OSNAME),openbsd)
release-shared:	LDSEARCH = -L${STLPORT_LIB_DIR} -L${CoMT_LIB_DIR} -Wl,-R${CoMT_LIB_DIR}
stldbg-shared:	LDSEARCH = -L${STLPORT_LIB_DIR} -L${CoMT_LIB_DIR_STLDBG} -Wl,-R${CoMT_LIB_DIR_STLDBG}
dbg-shared:	LDSEARCH = -L${STLPORT_LIB_DIR} -L${CoMT_LIB_DIR_DBG} -Wl,-R${CoMT_LIB_DIR_DBG}
endif

release-shared : LDLIBS = -lboost_regex_gcc -lstlport_gcc
stldbg-shared  : LDLIBS = -lstlport_gcc_stldebug -lboost_regex_gcc_stl-g
dbg-shared     : LDLIBS = -lboost_regex_gcc-g -lstlport_gcc_debug

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

ifeq ($(OSNAME),openbsd)
release-shared:	LDLIBS = -Wl,-Bstatic -lstlport_gcc -Wl,-Bdynamic -lxmt_gcc -lboost_test_utf_gcc -Wl,-Bstatic -lstlport_gcc -Wl,-Bdynamic
endif


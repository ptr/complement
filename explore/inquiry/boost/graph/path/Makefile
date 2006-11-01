# -*- Makefile -*- Time-stamp: <03/11/21 08:02:55 ptr>

SRCROOT := ../../../..
COMPILER_NAME := gcc

include Makefile.inc
include ${SRCROOT}/Makefiles/top.mak


INCLUDES += -I$(SRCROOT)/include -I$(BOOST_INCLUDE_DIR)

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

ifeq ($(OSNAME),openbsd)
release-shared:	LDLIBS = -Wl,-Bstatic -lstlport -Wl,-Bdynamic -lxmt_gcc -lboost_test_utf_gcc -Wl,-Bstatic -lstlport -Wl,-Bdynamic
endif


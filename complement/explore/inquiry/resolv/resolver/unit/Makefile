# -*- Makefile -*- Time-stamp: <05/11/03 19:15:02 ptr>
# $Id$

SRCROOT := ../../../..
COMPILER_NAME := gcc

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

INCLUDES += -I../.. -I$(BOOST_INCLUDE_DIR)

ifeq ($(OSNAME),linux)
release-shared:	LDSEARCH += -L${CoMT_LIB_DIR} -L../obj/gcc/shared -Wl,--rpath=${CoMT_LIB_DIR}:${STLPORT_LIB_DIR}:../obj/gcc/shared
stldbg-shared:	LDSEARCH += -L${CoMT_LIB_DIR} -L../obj/gcc/shared-stlg -Wl,--rpath=${CoMT_LIB_DIR}:${STLPORT_LIB_DIR}:../obj/gcc/shared-stlg
dbg-shared:	LDSEARCH += -L${CoMT_LIB_DIR} -L../obj/gcc/shared-g -Wl,--rpath=${CoMT_LIB_DIR}:${STLPORT_LIB_DIR}:../obj/gcc/shared-g
endif

ifeq ($(OSNAME),openbsd)
release-shared:	LDSEARCH += -L${CoMT_LIB_DIR} -Wl,-R${CoMT_LIB_DIR}:${STLPORT_LIB_DIR}
stldbg-shared:	LDSEARCH += -L${CoMT_LIB_DIR_STLDBG} -Wl,-R${CoMT_LIB_DIR_STLDBG}:${STLPORT_LIB_DIR}
dbg-shared:	LDSEARCH += -L${CoMT_LIB_DIR_DBG} -Wl,-R${CoMT_LIB_DIR_DBG}:${STLPORT_LIB_DIR}
endif

release-shared : LDLIBS = -lboost_test_utf -lboost_regex -lresolver -lresolv
stldbg-shared  : LDLIBS = -lboost_test_utfstlg -lboost_regexstlg -lresolverstlg -lresolv
dbg-shared     : LDLIBS = -lboost_test_utfg -lboost_regexg -lresolverg -lresolv

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



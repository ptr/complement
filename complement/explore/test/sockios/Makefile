# -*- Makefile -*- Time-stamp: <03/08/15 12:46:45 ptr>

SRCROOT := ../..
COMPILER_NAME := gcc

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak


INCLUDES += -I$(SRCROOT)/include -I$(BOOST_INCLUDE_DIR)

LIBMT_DIR = ${CoMT_DIR}/lib/mt
LIBSOCK_DIR = ${CoMT_DIR}/lib/sockios
LIBUTF_DIR = ${CoMT_DIR}/../extern/custom/boost/libs/test/unit_test_framework

ifeq ($(OSNAME),linux)
release-shared:	LDSEARCH += -L${LIBMT_DIR}/${OUTPUT_DIR} -L${LIBUTF_DIR}/${OUTPUT_DIR} -L${LIBSOCK_DIR}/${OUTPUT_DIR} -Wl,--rpath=${LIBMT_DIR}/${OUTPUT_DIR}:${LIBUTF_DIR}/${OUTPUT_DIR}:${LIBSOCK_DIR}/${OUTPUT_DIR}:${STLPORT_LIB_DIR}
stldbg-shared:	LDSEARCH += -L${LIBMT_DIR}/${OUTPUT_DIR_STLDBG} -L${LIBUTF_DIR}/${OUTPUT_DIR_STLDBG} -L${LIBSOCK_DIR}/${OUTPUT_DIR_STLDBG} -Wl,--rpath=${LIBMT_DIR}/${OUTPUT_DIR_STLDBG}:${LIBUTF_DIR}/${OUTPUT_DIR_STLDBG}:${LIBSOCK_DIR}/${OUTPUT_DIR_STLDBG}:${STLPORT_LIB_DIR}
dbg-shared:	LDSEARCH += -L${LIBMT_DIR}/${OUTPUT_DIR_DBG} -L${LIBUTF_DIR}/${OUTPUT_DIR_DBG} -L${LIBSOCK_DIR}/${OUTPUT_DIR_DBG} -Wl,--rpath=${LIBMT_DIR}/${OUTPUT_DIR_DBG}:${LIBUTF_DIR}/${OUTPUT_DIR_DBG}:${LIBSOCK_DIR}/${OUTPUT_DIR_DBG}:${STLPORT_LIB_DIR}
endif

release-shared : LDLIBS = -lxmt -lsockios -lboost_test_utf
stldbg-shared  : LDLIBS = -lxmtstlg -lsockiosstlg -lboost_test_utfstlg
dbg-shared     : LDLIBS = -lxmtg -lsockiosg -lboost_test_utfg

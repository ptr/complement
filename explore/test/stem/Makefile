# -*- Makefile -*- Time-stamp: <07/02/07 12:28:46 ptr>

SRCROOT := ../..
COMPILER_NAME := gcc

EXTRA_POST := dl-rel
EXTRA_POST_DBG := dl-dbg
EXTRA_POST_STLDBG := dl-stldbg

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

INCLUDES += -I$(SRCROOT)/include -I$(BOOST_INCLUDE_DIR)

LIBMT_DIR = ${CoMT_DIR}/lib/mt
LIBSOCK_DIR = ${CoMT_DIR}/lib/sockios
LIBSTEM_DIR = ${CoMT_DIR}/lib/stem
LIBUTF_DIR = ${CoMT_DIR}/../extern/custom/boost/libs/test/unit_test_framework

ifeq ($(OSNAME),linux)
release-shared:	LDSEARCH += -L${LIBMT_DIR}/${OUTPUT_DIR} -L${LIBUTF_DIR}/${OUTPUT_DIR} -L${LIBSOCK_DIR}/${OUTPUT_DIR} -L${LIBSTEM_DIR}/${OUTPUT_DIR} -Wl,--rpath=./dl/${OUTPUT_DIR}:${LIBMT_DIR}/${OUTPUT_DIR}:${LIBUTF_DIR}/${OUTPUT_DIR}:${LIBSOCK_DIR}/${OUTPUT_DIR}:${LIBSTEM_DIR}/${OUTPUT_DIR}:${STLPORT_LIB_DIR}
stldbg-shared:  LDSEARCH += -L${LIBMT_DIR}/${OUTPUT_DIR_STLDBG} -L${LIBUTF_DIR}/${OUTPUT_DIR_STLDBG} -L${LIBSOCK_DIR}/${OUTPUT_DIR_STLDBG} -L${LIBSTEM_DIR}/${OUTPUT_DIR_STLDBG} -Wl,--rpath=./dl/${OUTPUT_DIR_STLDBG}:${LIBMT_DIR}/${OUTPUT_DIR_STLDBG}:${LIBUTF_DIR}/${OUTPUT_DIR_STLDBG}:${LIBSOCK_DIR}/${OUTPUT_DIR_STLDBG}:${LIBSTEM_DIR}/${OUTPUT_DIR_STLDBG}:${STLPORT_LIB_DIR}
dbg-shared:     LDSEARCH += -L${LIBMT_DIR}/${OUTPUT_DIR_DBG} -L${LIBUTF_DIR}/${OUTPUT_DIR_DBG} -L${LIBSOCK_DIR}/${OUTPUT_DIR_DBG} -L${LIBSTEM_DIR}/${OUTPUT_DIR_DBG} -Wl,--rpath=./dl/${OUTPUT_DIR_DBG}:${LIBMT_DIR}/${OUTPUT_DIR_DBG}:${LIBUTF_DIR}/${OUTPUT_DIR_DBG}:${LIBSOCK_DIR}/${OUTPUT_DIR_DBG}:${LIBSTEM_DIR}/${OUTPUT_DIR_DBG}:${STLPORT_LIB_DIR}
endif

release-shared : LDLIBS = -lxmt -lsockios -lstem -lboost_test_utf -ldl
stldbg-shared  : LDLIBS = -lxmtstlg -lsockiosstlg -lstemstlg -lboost_test_utfstlg -ldl
dbg-shared     : LDLIBS = -lxmtg -lsockiosg -lstemg -lboost_test_utfg -ldl

# dbg-shared:	DEFS += -DDEBUG

PHONY += dl-rel dl-dbg dl-stldbg

dl-rel:
	${MAKE} -C dl release-shared

dl-dbg:
	${MAKE} -C dl dbg-shared

dl-stldbg:
	${MAKE} -C dl stldbg-shared

depend clean distclean mostlyclean maintainer-clean::
	${MAKE} -C dl $@

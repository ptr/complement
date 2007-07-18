# -*- Makefile -*- Time-stamp: <07/02/07 12:28:46 ptr>

SRCROOT := ../..
COMPILER_NAME := gcc

EXTRA_POST := dl-rel
EXTRA_POST_DBG := dl-dbg
EXTRA_POST_STLDBG := dl-stldbg

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

INCLUDES += -I$(SRCROOT)/include

LIBMT_DIR = ${CoMT_DIR}/lib/mt
LIBSOCK_DIR = ${CoMT_DIR}/lib/sockios
LIBSTEM_DIR = ${CoMT_DIR}/lib/stem
LIBEXAM_DIR = ${CoMT_DIR}/lib/exam

ifeq ($(OSNAME),linux)
release-shared:	LDSEARCH += -L${LIBMT_DIR}/${OUTPUT_DIR} -L${LIBEXAM_DIR}/${OUTPUT_DIR} -L${LIBSOCK_DIR}/${OUTPUT_DIR} -L${LIBSTEM_DIR}/${OUTPUT_DIR} -Wl,--rpath=./dl/${OUTPUT_DIR}:${LIBMT_DIR}/${OUTPUT_DIR}:${LIBEXAM_DIR}/${OUTPUT_DIR}:${LIBSOCK_DIR}/${OUTPUT_DIR}:${LIBSTEM_DIR}/${OUTPUT_DIR}:${STLPORT_LIB_DIR}
stldbg-shared:  LDSEARCH += -L${LIBMT_DIR}/${OUTPUT_DIR_STLDBG} -L${LIBEXAM_DIR}/${OUTPUT_DIR_STLDBG} -L${LIBSOCK_DIR}/${OUTPUT_DIR_STLDBG} -L${LIBSTEM_DIR}/${OUTPUT_DIR_STLDBG} -Wl,--rpath=./dl/${OUTPUT_DIR_STLDBG}:${LIBMT_DIR}/${OUTPUT_DIR_STLDBG}:${LIBEXAM_DIR}/${OUTPUT_DIR_STLDBG}:${LIBSOCK_DIR}/${OUTPUT_DIR_STLDBG}:${LIBSTEM_DIR}/${OUTPUT_DIR_STLDBG}:${STLPORT_LIB_DIR}
dbg-shared:     LDSEARCH += -L${LIBMT_DIR}/${OUTPUT_DIR_DBG} -L${LIBEXAM_DIR}/${OUTPUT_DIR_DBG} -L${LIBSOCK_DIR}/${OUTPUT_DIR_DBG} -L${LIBSTEM_DIR}/${OUTPUT_DIR_DBG} -Wl,--rpath=./dl/${OUTPUT_DIR_DBG}:${LIBMT_DIR}/${OUTPUT_DIR_DBG}:${LIBEXAM_DIR}/${OUTPUT_DIR_DBG}:${LIBSOCK_DIR}/${OUTPUT_DIR_DBG}:${LIBSTEM_DIR}/${OUTPUT_DIR_DBG}:${STLPORT_LIB_DIR}
endif

release-shared : LDLIBS = -lxmt -lsockios -lstem -lexam -ldl
stldbg-shared  : LDLIBS = -lxmtstlg -lsockiosstlg -lstemstlg -lexamstlg -ldl
dbg-shared     : LDLIBS = -lxmtg -lsockiosg -lstemg -lexamg -ldl

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

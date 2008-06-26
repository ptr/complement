# -*- Makefile -*- Time-stamp: <08/06/12 15:23:48 ptr>

SRCROOT := ../../..

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak


INCLUDES += -I$(SRCROOT)/include
ifdef BOOST_DIR
INCLUDES += -I$(BOOST_INCLUDE_DIR)
endif
DEFS += -D__FIT_EXAM

LIBMT_DIR = ${CoMT_DIR}/lib/mt
LIBSOCK_DIR = ${CoMT_DIR}/lib/sockios
LIBEXAM_DIR = ${CoMT_DIR}/lib/exam
# LIBUTF_DIR = ${CoMT_DIR}/../extern/custom/boost/libs/test/unit_test_framework

ifeq ($(OSNAME),linux)
release-shared:	LDFLAGS += -L${LIBMT_DIR}/${OUTPUT_DIR} -L${LIBEXAM_DIR}/${OUTPUT_DIR} -L${LIBSOCK_DIR}/${OUTPUT_DIR} -Wl,--rpath=${LIBMT_DIR}/${OUTPUT_DIR}:${LIBEXAM_DIR}/${OUTPUT_DIR}:${LIBSOCK_DIR}/${OUTPUT_DIR}:${STLPORT_LIB_DIR}
ifndef WITHOUT_STLPORT
stldbg-shared:	LDFLAGS += -L${LIBMT_DIR}/${OUTPUT_DIR_STLDBG} -L${LIBEXAM_DIR}/${OUTPUT_DIR_STLDBG} -L${LIBSOCK_DIR}/${OUTPUT_DIR_STLDBG} -Wl,--rpath=${LIBMT_DIR}/${OUTPUT_DIR_STLDBG}:${LIBEXAM_DIR}/${OUTPUT_DIR_STLDBG}:${LIBSOCK_DIR}/${OUTPUT_DIR_STLDBG}:${STLPORT_LIB_DIR}
endif
dbg-shared:	LDFLAGS += -L${LIBMT_DIR}/${OUTPUT_DIR_DBG} -L${LIBEXAM_DIR}/${OUTPUT_DIR_DBG} -L${LIBSOCK_DIR}/${OUTPUT_DIR_DBG} -Wl,--rpath=${LIBMT_DIR}/${OUTPUT_DIR_DBG}:${LIBEXAM_DIR}/${OUTPUT_DIR_DBG}:${LIBSOCK_DIR}/${OUTPUT_DIR_DBG}:${STLPORT_LIB_DIR}
endif

release-shared : LDLIBS = -lxmt -lsockios -lexam
ifndef WITHOUT_STLPORT
stldbg-shared  : LDLIBS = -lxmtstlg -lsockiosstlg -lexamstlg
endif
dbg-shared     : LDLIBS = -lxmtg -lsockiosg -lexamg

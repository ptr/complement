# -*- Makefile -*- Time-stamp: <09/02/04 10:56:00 ptr>

SRCROOT := ../../..

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak


INCLUDES += -I$(SRCROOT)/include
ifdef BOOST_DIR
INCLUDES += -I$(BOOST_INCLUDE_DIR)
endif
DEFS += -D__FIT_EXAM

LIBMT_DIR = ${CoMT_DIR}/lib/mt
LIBEXAM_DIR = ${CoMT_DIR}/lib/exam
LIBMISC_DIR = ${CoMT_DIR}/lib/misc

ifeq ($(OSNAME),linux)
release-shared:	LDFLAGS += -L${LIBMT_DIR}/${OUTPUT_DIR} -L${LIBEXAM_DIR}/${OUTPUT_DIR} -L${LIBMISC_DIR}/${OUTPUT_DIR} -Wl,--rpath=${LIBMT_DIR}/${OUTPUT_DIR}:${LIBEXAM_DIR}/${OUTPUT_DIR}:${LIBMISC_DIR}/${OUTPUT_DIR}:${STLPORT_LIB_DIR}
ifndef WITHOUT_STLPORT
stldbg-shared:	LDFLAGS += -L${LIBMT_DIR}/${OUTPUT_DIR_STLDBG} -L${LIBEXAM_DIR}/${OUTPUT_DIR_STLDBG} -L${LIBMISC_DIR}/${OUTPUT_DIR_STLDBG} -Wl,--rpath=${LIBMT_DIR}/${OUTPUT_DIR_STLDBG}:${LIBEXAM_DIR}/${OUTPUT_DIR_STLDBG}:${LIBMISC_DIR}/${OUTPUT_DIR_STLDBG}:${STLPORT_LIB_DIR}
endif
dbg-shared:	LDFLAGS += -L${LIBMT_DIR}/${OUTPUT_DIR_DBG} -L${LIBEXAM_DIR}/${OUTPUT_DIR_DBG} -L${LIBMISC_DIR}/${OUTPUT_DIR_DBG} -Wl,--rpath=${LIBMT_DIR}/${OUTPUT_DIR_DBG}:${LIBEXAM_DIR}/${OUTPUT_DIR_DBG}:${LIBMISC_DIR}/${OUTPUT_DIR_DBG}:${STLPORT_LIB_DIR}
endif

release-shared : LDLIBS = -lxmt -lexam -lmisc
ifndef WITHOUT_STLPORT
stldbg-shared  : LDLIBS = -lxmtstlg -lexamstlg -lmiscstlg
endif
dbg-shared     : LDLIBS = -lxmtg -lexamg -lmiscg

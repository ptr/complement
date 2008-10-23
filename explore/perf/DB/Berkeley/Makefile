# -*- Makefile -*- Time-stamp: <08/08/14 21:19:55 yeti>

SRCROOT := ../../..

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak


INCLUDES += -I$(SRCROOT)/include
INCLUDES += -I..
ifdef BOOST_DIR
INCLUDES += -I$(BOOST_INCLUDE_DIR)
endif
DEFS += -D__FIT_EXAM

LIBMT_DIR = ${SRCROOT}/lib/mt
LIBMISC_DIR = ${SRCROOT}/lib/misc
LIBEXAM_DIR = ${SRCROOT}/lib/exam

ifeq ($(OSNAME),linux)

release-shared:	LDFLAGS += -L${LIBMT_DIR}/${OUTPUT_DIR} \
 -L${LIBEXAM_DIR}/${OUTPUT_DIR} \
 -L${LIBMISC_DIR}/${OUTPUT_DIR} \
 -Wl,--rpath=${LIBMT_DIR}/${OUTPUT_DIR}:${LIBEXAM_DIR}/${OUTPUT_DIR}:${LIBMISC_DIR}/${OUTPUT_DIR}:${STLPORT_LIB_DIR}

dbg-shared:	LDFLAGS += -L${LIBMT_DIR}/${OUTPUT_DIR_DBG} \
 -L${LIBEXAM_DIR}/${OUTPUT_DIR_DBG} \
 -L${LIBMISC_DIR}/${OUTPUT_DIR_DBG} \
 -Wl,--rpath=${LIBMT_DIR}/${OUTPUT_DIR_DBG}:${LIBEXAM_DIR}/${OUTPUT_DIR_DBG}:${LIBMISC_DIR}/${OUTPUT_DIR_DBG}:${STLPORT_LIB_DIR}

ifndef WITHOUT_STLPORT
stldbg-shared:	LDFLAGS += -L${LIBMT_DIR}/${OUTPUT_DIR_STLDBG} \
 -L${LIBEXAM_DIR}/${OUTPUT_DIR_STLDBG} \
 -L${LIBMISC_DIR}/${OUTPUT_DIR_STLDBG} \
 -Wl,--rpath=${LIBMT_DIR}/${OUTPUT_DIR_STLDBG}:${LIBEXAM_DIR}/${OUTPUT_DIR_STLDBG}:${LIBMISC_DIR}/${OUTPUT_DIR_STLDBG}:${STLPORT_LIB_DIR}
endif

endif

release-shared : LDLIBS = -lxmt -lexam -lmisc -ldb_cxx -ldb
dbg-shared     : LDLIBS = -lxmtg -lexamg -lmiscg -ldb_cxx -ldb
ifndef WITHOUT_STLPORT
stldbg-shared  : LDLIBS = -lxmtstlg -lexamstlg -lmiscstlg -ldb_cxx -ldb
endif

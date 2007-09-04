# -*- Makefile -*- Time-stamp: <07/09/03 22:27:45 ptr>

SRCROOT := ../../..

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

INCLUDES += -I${CoMT_INCLUDE_DIR}
DEFS += -D__FIT_EXAM

LIBEXAM_DIR = ${CoMT_DIR}/lib/exam
LIBXMT_DIR = ${CoMT_DIR}/lib/mt

LDFLAGS += -Wl,-rpath=${STLPORT_LIB_DIR}

ifeq ($(OSNAME),linux)

release-shared:	LDSEARCH += -L${LIBEXAM_DIR}/${OUTPUT_DIR} -L$(LIBXMT_DIR)/${OUTPUT_DIR} -Wl,--rpath=${LIBEXAM_DIR}/${OUTPUT_DIR}:$(LIBXMT_DIR)/${OUTPUT_DIR}:${STLPORT_LIB_DIR}

dbg-shared:	LDSEARCH += -L${LIBEXAM_DIR}/${OUTPUT_DIR_DBG} -L$(LIBXMT_DIR)/${OUTPUT_DIR_DBG} -Wl,--rpath=${LIBEXAM_DIR}/${OUTPUT_DIR_DBG}:$(LIBXMT_DIR)/${OUTPUT_DIR_DBG}:${STLPORT_LIB_DIR}

ifndef WITHOUT_STLPORT
stldbg-shared:	LDSEARCH += -L${LIBEXAM_DIR}/${OUTPUT_DIR_STLDBG} -L$(LIBXMT_DIR)/${OUTPUT_DIR_STLDBG} -Wl,--rpath=${LIBEXAM_DIR}/${OUTPUT_DIR_STLDBG}:$(LIBXMT_DIR)/${OUTPUT_DIR_STLDBG}:${STLPORT_LIB_DIR}
endif

endif

release-shared : LDLIBS = -lexam -lxmt
dbg-shared     : LDLIBS = -lexamg -lxmtg
ifndef WITHOUT_STLPORT
stldbg-shared  : LDLIBS = -lexamstlg -lxmtstlg
endif

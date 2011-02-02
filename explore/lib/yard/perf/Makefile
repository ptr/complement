# -*- Makefile -*- Time-stamp: <2011-02-02 16:27:27 ptr>

SRCROOT := ../../..

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak


INCLUDES += -I$(SRCROOT)/include
ifdef BOOST_DIR
INCLUDES += -I$(BOOST_INCLUDE_DIR)
endif
DEFS += -D__FIT_EXAM -D__FIT_YARD_UT

LIBMT_DIR = ${CoMT_DIR}/lib/mt
LIBEXAM_DIR = ${CoMT_DIR}/lib/exam
LIBMISC_DIR = ${CoMT_DIR}/lib/misc

#CXXFLAGS += -pg -fprofile-arcs -ftest-coverage

ifeq ($(OSNAME),linux)
release-shared:	LDFLAGS += -L${LIBMT_DIR}/${OUTPUT_DIR} -L${LIBEXAM_DIR}/${OUTPUT_DIR} \
                           -L${LIBMISC_DIR}/${OUTPUT_DIR} -L../${OUTPUT_DIR} \
                           -Wl,--rpath=${LIBMT_DIR}/${OUTPUT_DIR}:${LIBEXAM_DIR}/${OUTPUT_DIR}:${LIBMISC_DIR}/${OUTPUT_DIR}:../${OUTPUT_DIR}:${STLPORT_LIB_DIR}

dbg-shared:	LDFLAGS += -L${LIBMT_DIR}/${OUTPUT_DIR_DBG} -L${LIBEXAM_DIR}/${OUTPUT_DIR_DBG} \
                           -L${LIBMISC_DIR}/${OUTPUT_DIR_DBG} -L../${OUTPUT_DIR_DBG} \
                           -Wl,--rpath=${LIBMT_DIR}/${OUTPUT_DIR_DBG}:${LIBEXAM_DIR}/${OUTPUT_DIR_DBG}:${LIBMISC_DIR}/${OUTPUT_DIR_DBG}:../${OUTPUT_DIR_DBG}:${STLPORT_LIB_DIR}

ifndef WITHOUT_STLPORT
stldbg-shared:	LDFLAGS += -L${LIBMT_DIR}/${OUTPUT_DIR_STLDBG} -L${LIBEXAM_DIR}/${OUTPUT_DIR_STLDBG} \
                           -L${LIBMISC_DIR}/${OUTPUT_DIR_STLDBG} -L../${OUTPUT_DIR_STLDBG} \
                           -Wl,--rpath=${LIBMT_DIR}/${OUTPUT_DIR_STLDBG}:${LIBEXAM_DIR}/${OUTPUT_DIR_STLDBG}:${LIBMISC_DIR}/${OUTPUT_DIR_STLDBG}:../${OUTPUT_DIR_STLDBG}:${STLPORT_LIB_DIR}
endif

endif

release-shared : LDLIBS = -lyard -lxmt -lexam -lmisc

dbg-shared     : LDLIBS = -lyardg -lxmtg -lexamg -lmiscg

ifndef WITHOUT_STLPORT
stldbg-shared  : LDLIBS = -lyardstlg -lxmtstlg -lexamstlg -lmiscstlg
endif

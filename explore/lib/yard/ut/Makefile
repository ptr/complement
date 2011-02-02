# -*- Makefile -*- Time-stamp: <2011-02-02 16:06:06 ptr>

SRCROOT := ../../..

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

INCLUDES += -I$(SRCROOT)/include
DEFS += -D__FIT_EXAM -D__FIT_YARD_UT

LIBMT_DIR = ${CoMT_DIR}/lib/mt
LIBEXAM_DIR = ${CoMT_DIR}/lib/exam
LIBMISC_DIR = ${CoMT_DIR}/lib/misc

ifeq ($(OSNAME),linux)

release-shared:	LDFLAGS += -L${LIBMT_DIR}/${OUTPUT_DIR} -L${LIBEXAM_DIR}/${OUTPUT_DIR} \
                           -L${LIBMISC_DIR}/${OUTPUT_DIR} -L../${OUTPUT_DIR} \
                           -Wl,--rpath=${LIBMT_DIR}/${OUTPUT_DIR}:${LIBEXAM_DIR}/${OUTPUT_DIR}:${LIBMISC_DIR}/${OUTPUT_DIR}:../${OUTPUT_DIR}:${STLPORT_LIB_DIR}

dbg-shared:     LDFLAGS += -L${LIBMT_DIR}/${OUTPUT_DIR_DBG} -L${LIBEXAM_DIR}/${OUTPUT_DIR_DBG} \
                           -L${LIBMISC_DIR}/${OUTPUT_DIR_DBG} -L../${OUTPUT_DIR_DBG} \
                           -Wl,--rpath=${LIBMT_DIR}/${OUTPUT_DIR_DBG}:${LIBEXAM_DIR}/${OUTPUT_DIR_DBG}:${LIBMISC_DIR}/${OUTPUT_DIR_DBG}:../${OUTPUT_DIR_DBG}:${STLPORT_LIB_DIR}

ifndef WITHOUT_STLPORT

stldbg-shared:  LDFLAGS += -L${LIBMT_DIR}/${OUTPUT_DIR_STLDBG} -L${LIBEXAM_DIR}/${OUTPUT_DIR_STLDBG} \
                           -L${LIBMISC_DIR}/${OUTPUT_DIR_STLDBG} -L../${OUTPUT_DIR_STLDBG} \
                           -Wl,--rpath=${LIBMT_DIR}/${OUTPUT_DIR_STLDBG}:${LIBEXAM_DIR}/${OUTPUT_DIR_STLDBG}:${LIBMISC_DIR}/${OUTPUT_DIR_STLDBG}:../${OUTPUT_DIR_STLDBG}:${STLPORT_LIB_DIR}

endif

endif

release-shared : LDLIBS = -lyard -lxmt -lexam -lmisc
dbg-shared     : LDLIBS = -lyardg -lxmtg -lexamg -lmiscg
ifndef WITHOUT_STLPORT
stldbg-shared  : LDLIBS = -lyardstlg -lxmtstlg -lexamstlg -lmiscstlg
endif

check-release-shared:	release-shared
	@${PRG}

check-dbg-shared:	dbg-shared
	@${PRG_DBG}

ifndef WITHOUT_STLPORT
check-stldbg-shared:	stldbg-shared
	@${PRG_STLDBG}
endif

check:	all-shared
	@${PRG}
	@${PRG_DBG}
ifndef WITHOUT_STLPORT
	@${PRG_STLDBG}
endif

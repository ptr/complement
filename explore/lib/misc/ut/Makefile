# -*- Makefile -*-

SRCROOT := ../../..

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak


INCLUDES += -I$(SRCROOT)/include
DEFS += -D__FIT_EXAM

LIBEXAM_DIR = ${CoMT_DIR}/lib/exam
LIBXMT_DIR = ${CoMT_DIR}/lib/mt
LIBMISC_DIR = ${CoMT_DIR}/lib/misc

ifeq ($(OSNAME),linux)

release-shared:	LDFLAGS += -L${LIBEXAM_DIR}/${OUTPUT_DIR} -L${LIBXMT_DIR}/${OUTPUT_DIR} -L${LIBMISC_DIR}/${OUTPUT_DIR} -Wl,--rpath=${LIBEXAM_DIR}/${OUTPUT_DIR}:${LIBXMT_DIR}/${OUTPUT_DIR}:${LIBMISC_DIR}/${OUTPUT_DIR}:${STLPORT_LIB_DIR}

dbg-shared:	LDFLAGS += -L${LIBEXAM_DIR}/${OUTPUT_DIR_DBG} -L${LIBXMT_DIR}/${OUTPUT_DIR_DBG} -L${LIBMISC_DIR}/${OUTPUT_DIR_DBG} -Wl,--rpath=${LIBEXAM_DIR}/${OUTPUT_DIR_DBG}:${LIBXMT_DIR}/${OUTPUT_DIR_DBG}:${LIBMISC_DIR}/${OUTPUT_DIR_DBG}:${STLPORT_LIB_DIR}

ifndef WITHOUT_STLPORT
stldbg-shared:	LDFLAGS += -L${LIBEXAM_DIR}/${OUTPUT_DIR_STLDBG} -L${LIBXMT_DIR}/${OUTPUT_DIR_STLDBG} -L${LIBMISC_DIR}/${OUTPUT_DIR_STLDBG} -Wl,--rpath=${LIBEXAM_DIR}/${OUTPUT_DIR_STLDBG}:${LIBXMT_DIR}/${OUTPUT_DIR_STLDBG}:${LIBMISC_DIR}/${OUTPUT_DIR_STLDBG}:${STLPORT_LIB_DIR}
endif

endif

ifeq ($(OSNAME),openbsd)

ifndef WITHOUT_STLPORT
release-shared:	LDFLAGS += -Wl,-R:${STLPORT_LIB_DIR}
dbg-shared:	LDFLAGS += -Wl,-R:${STLPORT_LIB_DIR}
stldbg-shared:	LDFLAGS += -Wl,-R:${STLPORT_LIB_DIR}
endif

endif

LDLIBS += -lexam -lmisc
misc_ut_LDLIBS += -lexam -lmisc
opts_ut_LDLIBS += -lexam -lmisc
LDLIBS_DBG += -lexamg -lmiscg
misc_ut_LDLIBS_DBG += -lexamg -lmiscg
opts_ut_LDLIBS_DBG += -lexamg -lmiscg
ifndef WITHOUT_STLPORT
LDLIBS_STLDBG += -lexamstlg -lmiscstlg
misc_ut_LDLIBS_STLDBG += -lexamstlg -lmiscstlg
opts_ut_LDLIBS_STLDBG += -lexamstlg -lmiscstlg
endif

ifeq ($(OSNAME),freebsd)
STDLIBS += -lthr
endif

ifeq ($(OSNAME),sunos)
STDLIBS += -lrt
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


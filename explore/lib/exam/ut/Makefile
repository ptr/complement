# -*- Makefile -*-

SRCROOT := ../../..

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

INCLUDES += -I${CoMT_INCLUDE_DIR}
DEFS += -D__FIT_EXAM

LIBEXAM_DIR = ${CoMT_DIR}/lib/exam
LIBMISC_DIR = ${CoMT_DIR}/lib/misc

CXXFLAGS += -Wall

ifeq ($(OSNAME),linux)

release-shared:	LDFLAGS += -L${LIBEXAM_DIR}/${OUTPUT_DIR} -Wl,--rpath=${LIBEXAM_DIR}/${OUTPUT_DIR}:$(LIBMISC_DIR)/${OUTPUT_DIR}:${STLPORT_LIB_DIR}

dbg-shared:	LDFLAGS += -L${LIBEXAM_DIR}/${OUTPUT_DIR_DBG} -Wl,--rpath=${LIBEXAM_DIR}/${OUTPUT_DIR_DBG}:$(LIBMISC_DIR)/${OUTPUT_DIR_DBG}:${STLPORT_LIB_DIR}

ifndef WITHOUT_STLPORT
stldbg-shared:	LDFLAGS += -L${LIBEXAM_DIR}/${OUTPUT_DIR_STLDBG} -L$(LIBXMT_DIR)/${OUTPUT_DIR_STLDBG} -Wl,--rpath=${LIBEXAM_DIR}/${OUTPUT_DIR_STLDBG}:$(LIBMISC_DIR)/${OUTPUT_DIR_STLDBG}:$(LIBXMT_DIR)/${OUTPUT_DIR_STLDBG}:${STLPORT_LIB_DIR}
endif

endif

LDLIBS = -lexam
LDLIBS_DBG = -lexamg

ifndef WITHOUT_STLPORT
stldbg-shared  : LDLIBS = -lexamstlg -lxmtstlg
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

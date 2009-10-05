# -*- Makefile -*- Time-stamp: <09/10/05 17:12:59 ptr>

SRCROOT := ../../..
COMPILER_NAME := gcc
# ALL_TAGS := install-release-shared install-dbg-shared
# CoMT_DIR := ../../external/complement/explore

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

# DEFS += -DUNIT_TEST
INCLUDES += -I${CoMT_INCLUDE_DIR} -I..
DEFS += -D__FIT_EXAM

LIBEXAM_DIR = ../../exam
LIBXMT_DIR = ../../mt
LIBMISC_DIR = ../../misc
LIBSOCKIOS_DIR = ../../sockios
LIBSTEM_DIR = ../../stem
LIBJANUS_DIR = ..

ifeq ($(OSNAME),linux)

release-shared: LDFLAGS += -L${LIBEXAM_DIR}/${OUTPUT_DIR} -L$(LIBXMT_DIR)/${OUTPUT_DIR} -L$(LIBSOCKIOS_DIR)/${OUTPUT_DIR} -L$(LIBSTEM_DIR)/${OUTPUT_DIR} -L$(LIBJANUS_DIR)/${OUTPUT_DIR} -L${LIBMISC_DIR}/${OUTPUT_DIR} -Wl,--rpath=${LIBEXAM_DIR}/${OUTPUT_DIR}:$(LIBXMT_DIR)/${OUTPUT_DIR}:${LIBMISC_DIR}/${OUTPUT_DIR}:${LIBSOCKIOS_DIR}/${OUTPUT_DIR}:${LIBSTEM_DIR}/${OUTPUT_DIR}:${LIBJANUS_DIR}/${OUTPUT_DIR}:${STLPORT_LIB_DIR}

dbg-shared:     LDFLAGS += -L${LIBEXAM_DIR}/${OUTPUT_DIR_DBG} -L$(LIBXMT_DIR)/${OUTPUT_DIR_DBG} -L$(LIBSOCKIOS_DIR)/${OUTPUT_DIR_DBG} -L$(LIBSTEM_DIR)/${OUTPUT_DIR_DBG} -L$(LIBJANUS_DIR)/${OUTPUT_DIR_DBG} -L${LIBMISC_DIR}/${OUTPUT_DIR_DBG} -Wl,--rpath=${LIBEXAM_DIR}/${OUTPUT_DIR_DBG}:$(LIBXMT_DIR)/${OUTPUT_DIR_DBG}:${LIBMISC_DIR}/${OUTPUT_DIR_DBG}:${LIBSOCKIOS_DIR}/${OUTPUT_DIR_DBG}:${LIBSTEM_DIR}/${OUTPUT_DIR_DBG}:${LIBJANUS_DIR}/${OUTPUT_DIR_DBG}:${STLPORT_LIB_DIR}

ifndef WITHOUT_STLPORT
stldbg-shared:  LDFLAGS += -L${LIBEXAM_DIR}/${OUTPUT_DIR_STLDBG} -L$(LIBXMT_DIR)/${OUTPUT_DIR_STLDBG} -L$(LIBSOCKIOS_DIR)/${OUTPUT_DIR_STLDBG} -L$(LIBSTEM_DIR)/${OUTPUT_DIR_STLDBG} -L$(LIBJANUS_DIR)/${OUTPUT_DIR_STLDBG} -L${LIBMISC_DIR}/${OUTPUT_DIR_STDLDBG} -Wl,--rpath=${LIBEXAM_DIR}/${OUTPUT_DIR_STLDBG}:$(LIBXMT_DIR)/${OUTPUT_DIR_STLDBG}:${LIBMISC_DIR}/${OUTPUT_DIR_STDLDBG}:${LIBSOCKIOS_DIR}/${OUTPUT_DIR_STLDBG}:${LIBSTEM_DIR}/${OUTPUT_DIR_STLDBG}:${LIBJANUS_DIR}/${OUTPUT_DIR_STLDBG}:${STLPORT_LIB_DIR}
endif

endif

release-shared: PROJECT_LIBS = -lxmt -lmisc -lsockios -lstem -lexam -ljanus
dbg-shared:     PROJECT_LIBS = -lxmtg -lmiscg -lsockiosg -lstemg -lexamg -ljanusg
ifndef WITHOUT_STLPORT
stldbg-shared:  PROJECT_LIBS = -lxmtstlg -lmiscstlg -lsockiosstlg -lstemstlg -lexamstlg -ljanusstlg
endif

dbg-shared:    DEFS += -D__FIT_VS_TRACE
ifndef WITHOUT_STLPORT
stldbg-shared: DEFS += -D__FIT_VS_TRACE
endif

LDLIBS = ${PROJECT_LIBS}

check:  all-shared
	${OUTPUT_DIR}/${PRGNAME} || exit 1
	${OUTPUT_DIR_DBG}/${PRGNAME} || exit 1
ifndef WITHOUT_STLPORT
	${OUTPUT_DIR_STLDBG}/${PRGNAME} || exit 1;
endif

check-release-shared:   release-shared
	${OUTPUT_DIR}/${PRGNAME} || exit 1

check-dbg-shared:       dbg-shared
	${OUTPUT_DIR_DBG}/${PRGNAME} || exit 1

ifndef WITHOUT_STLPORT
check-stldbg-shared:    stldbg-shared
	${OUTPUT_DIR_STLDBG}/${PRGNAME} || exit 1
endif

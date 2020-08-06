# -*- Makefile -*-

SRCROOT := ../../../..

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

LIBMT_DIR = ../../../mt
LIBMISC_DIR = ../../../misc
LIBSOCK_DIR = ../../../sockios


ifeq ($(OSNAME),linux)

release-shared: LDFLAGS += -L${LIBMT_DIR}/${OUTPUT_DIR} -L${LIBMISC_DIR}/${OUTPUT_DIR} -L${LIBSOCK_DIR}/${OUTPUT_DIR} -Wl,--rpath=${LIBMT_DIR}/${OUTPUT_DIR}:${LIBMISC_DIR}/${OUTPUT_DIR}:${LIBSOCK_DIR}/${OUTPUT_DIR}

ifndef WITHOUT_STLPORT

stldbg-shared:  LDFLAGS += -L${LIBMT_DIR}/${OUTPUT_DIR_STLDBG} -L${LIBMISC_DIR}/${OUTPUT_DIR_STLDBG} -L${LIBSOCK_DIR}/${OUTPUT_DIR_STLDBG} -Wl,--rpath=${LIBMT_DIR}/${OUTPUT_DIR_STLDBG}:${LIBMISC_DIR}/${OUTPUT_DIR_STLDBG}:${LIBSOCK_DIR}/${OUTPUT_DIR_STLDBG}

endif

dbg-shared:     LDFLAGS += -L${LIBMT_DIR}/${OUTPUT_DIR_DBG} -L${LIBMISC_DIR}/${OUTPUT_DIR_DBG} -L${LIBSOCK_DIR}/${OUTPUT_DIR_DBG} -Wl,--rpath=${LIBMT_DIR}/${OUTPUT_DIR_DBG}:${LIBMISC_DIR}/${OUTPUT_DIR_DBG}:${LIBSOCK_DIR}/${OUTPUT_DIR_DBG}

endif


INCLUDES += -I${CoMT_INCLUDE_DIR}

LDLIBS = -lxmt -lsockios -lmisc
LDLIBS_DBG = -lxmtg -lsockiosg -lmiscg

ifndef WITHOUT_STLPORT
LDLIBS_STLDBG = -lxmtstlg -lsockiosstlg -lmiscstlg
endif

# LDLIBS = ${PROJECT_LIBS}

check:	all-shared
	${OUTPUT_DIR}/${PRGNAME} || exit 1
	${OUTPUT_DIR_DBG}/${PRGNAME} || exit 1
ifndef WITHOUT_STLPORT
	${OUTPUT_DIR_STLDBG}/${PRGNAME} || exit 1;
endif

check-release-shared:	release-shared
	${OUTPUT_DIR}/${PRGNAME} || exit 1

check-dbg-shared:	dbg-shared
	${OUTPUT_DIR_DBG}/${PRGNAME} || exit 1

ifndef WITHOUT_STLPORT
check-stldbg-shared:	stldbg-shared
	${OUTPUT_DIR_STLDBG}/${PRGNAME} || exit 1
endif

# -*- Makefile -*- Time-stamp: <2011-10-05 23:35:02 ptr>

SRCROOT := ../../../..

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

LIBMT_DIR = ../../../mt
LIBMISC_DIR = ../../../misc
LIBSOCK_DIR = ../../../sockios
LIBNET_DIR = ../..
LIBBOOSTRE_DIR = ../../../../../extern/custom/boost/libs/regex

ifeq ($(OSNAME),linux)

release-shared: LDFLAGS += -L${LIBMT_DIR}/${OUTPUT_DIR} -L${LIBMISC_DIR}/${OUTPUT_DIR} -L${LIBSOCK_DIR}/${OUTPUT_DIR} -L${LIBBOOSTRE_DIR}/${OUTPUT_DIR} -L${LIBNET_DIR}/${OUTPUT_DIR} -Wl,--rpath=${LIBMT_DIR}/${OUTPUT_DIR}:${LIBMISC_DIR}/${OUTPUT_DIR}:${LIBSOCK_DIR}/${OUTPUT_DIR}:${LIBBOOSTRE_DIR}/${OUTPUT_DIR}:${LIBNET_DIR}/${OUTPUT_DIR}:${STLPORT_LIB_DIR}

ifndef WITHOUT_STLPORT

stldbg-shared:  LDFLAGS += -L${LIBMT_DIR}/${OUTPUT_DIR_STLDBG} -L${LIBMISC_DIR}/${OUTPUT_DIR_STLDBG} -L${LIBSOCK_DIR}/${OUTPUT_DIR_STLDBG} -L${LIBBOOSTRE_DIR}/${OUTPUT_DIR_STLDBG} -L${LIBNET_DIR}/${OUTPUT_DIR_STLDBG} -Wl,--rpath=${LIBMT_DIR}/${OUTPUT_DIR_STLDBG}:${LIBMISC_DIR}/${OUTPUT_DIR_STLDBG}:${LIBSOCK_DIR}/${OUTPUT_DIR_STLDBG}:${LIBBOOSTRE_DIR}/${OUTPUT_DIR_STLDBG}:${LIBNET_DIR}/${OUTPUT_DIR_STLDBG}:${STLPORT_LIB_DIR}

endif

dbg-shared:     LDFLAGS += -L${LIBMT_DIR}/${OUTPUT_DIR_DBG} -L${LIBMISC_DIR}/${OUTPUT_DIR_DBG} -L${LIBSOCK_DIR}/${OUTPUT_DIR_DBG} -L${LIBBOOSTRE_DIR}/${OUTPUT_DIR_DBG} -L${LIBNET_DIR}/${OUTPUT_DIR_DBG} -Wl,--rpath=${LIBMT_DIR}/${OUTPUT_DIR_DBG}:${LIBMISC_DIR}/${OUTPUT_DIR_DBG}:${LIBSOCK_DIR}/${OUTPUT_DIR_DBG}:${LIBBOOSTRE_DIR}/${OUTPUT_DIR_DBG}:${LIBNET_DIR}/${OUTPUT_DIR_DBG}:${STLPORT_LIB_DIR}

endif


INCLUDES += -I${CoMT_INCLUDE_DIR} -I${BOOST_INCLUDE_DIR}

release-shared: PROJECT_LIBS = -lxmt -lsockios -lNet -lboost_regex
dbg-shared:	PROJECT_LIBS = -lxmtg -lsockiosg -lNetg -lboost_regexg
ifndef WITHOUT_STLPORT
stldbg-shared:	PROJECT_LIBS = -lxmtstlg -lsockiosstlg -lNetstlg -lboost_regexstlg
endif

LDLIBS = ${PROJECT_LIBS}

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

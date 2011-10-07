# -*- Makefile -*- Time-stamp: <2011-10-07 09:17:32 ptr>

SRCROOT := ../../../../..
# CoMT_DIR := ../../external/complement/explore

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

DEFS += -D__FIT_EXAM

LIBMT_DIR = ${CoMT_DIR}/lib/mt
LIBSOCK_DIR = ${CoMT_DIR}/lib/sockios
LIBSTEM_DIR = ${CoMT_DIR}/lib/stem
LIBEXAM_DIR = ${CoMT_DIR}/lib/exam
LIBMISC_DIR = ${CoMT_DIR}/lib/misc
LIBNET_DIR = ${CoMT_DIR}/lib/net
LIBFCP_DIR = ..
LIBBOOSTFS_DIR = ${CoMT_DIR}/../extern/custom/boost/libs/filesystem
LIBBOOSTRE_DIR = ${CoMT_DIR}/../extern/custom/boost/libs/regex

ifeq ($(OSNAME),linux)

release-shared: LDFLAGS += -L${LIBEXAM_DIR}/${OUTPUT_DIR} -L${LIBBOOSTFS_DIR}/${OUTPUT_DIR} -L${LIBNET_DIR}/${OUTPUT_DIR} -L${LIBSTEM_DIR}/${OUTPUT_DIR} -L${LIBFCP_DIR}/${OUTPUT_DIR} -Wl,--rpath=${LIBMT_DIR}/${OUTPUT_DIR}:${LIBEXAM_DIR}/${OUTPUT_DIR}:${LIBMISC_DIR}/${OUTPUT_DIR}:${LIBSOCK_DIR}/${OUTPUT_DIR}:${LIBSTEM_DIR}/${OUTPUT_DIR}:${LIBBOOSTFS_DIR}/${OUTPUT_DIR}:${LIBBOOSTRE_DIR}/${OUTPUT_DIR}:${LIBNET_DIR}/${OUTPUT_DIR}:${LIBSTEM_DIR}/${OUTPUT_DIR}:${LIBFCP_DIR}/${OUTPUT_DIR}:${STLPORT_LIB_DIR}

ifndef WITHOUT_STLPORT

stldbg-shared:  LDFLAGS += -L${LIBEXAM_DIR}/${OUTPUT_DIR_STLDBG} -L${LIBBOOSTFS_DIR}/${OUTPUT_DIR_STLDBG} -L${LIBNET_DIR}/${OUTPUT_DIR_STLDBG} -L${LIBSTEM_DIR}/${OUTPUT_DIR_STLDBG} -L${LIBFCP_DIR}/${OUTPUT_DIR_STLDBG} -Wl,--rpath=${LIBMT_DIR}/${OUTPUT_DIR_STLDBG}:${LIBEXAM_DIR}/${OUTPUT_DIR_STLDBG}:${LIBMISC_DIR}/${OUTPUT_DIR_STLDBG}:${LIBSOCK_DIR}/${OUTPUT_DIR_STLDBG}:${LIBSTEM_DIR}/${OUTPUT_DIR_STLDBG}:${LIBBOOSTFS_DIR}/${OUTPUT_DIR_STLDBG}:${LIBBOOSTRE_DIR}/${OUTPUT_DIR_STLDBG}:${LIBNET_DIR}/${OUTPUT_DIR_STLDBG}:${LIBSTEM_DIR}/${OUTPUT_DIR_STLDBG}:${LIBFCP_DIR}/${OUTPUT_DIR_STLDBG}:${STLPORT_LIB_DIR}

endif

dbg-shared:     LDFLAGS += -L${LIBEXAM_DIR}/${OUTPUT_DIR_DBG} -L${LIBBOOSTFS_DIR}/${OUTPUT_DIR_DBG} -L${LIBNET_DIR}/${OUTPUT_DIR_DBG} -L${LIBSTEM_DIR}/${OUTPUT_DIR_DBG} -L${LIBFCP_DIR}/${OUTPUT_DIR_DBG} -Wl,--rpath=${LIBMT_DIR}/${OUTPUT_DIR_DBG}:${LIBEXAM_DIR}/${OUTPUT_DIR_DBG}:${LIBMISC_DIR}/${OUTPUT_DIR_DBG}:${LIBSOCK_DIR}/${OUTPUT_DIR_DBG}:${LIBSTEM_DIR}/${OUTPUT_DIR_DBG}:${LIBBOOSTFS_DIR}/${OUTPUT_DIR_DBG}:${LIBBOOSTRE_DIR}/${OUTPUT_DIR_DBG}:${LIBNET_DIR}/${OUTPUT_DIR_DBG}:${LIBSTEM_DIR}/${OUTPUT_DIR_DBG}:${LIBFCP_DIR}/${OUTPUT_DIR_DBG}:${STLPORT_LIB_DIR}

endif


INCLUDES += -I${CoMT_INCLUDE_DIR} -I${BOOST_INCLUDE_DIR}

release-shared: PROJECT_LIBS = -lNet -lboost_fs -lexam -lfcopy -lstem
dbg-shared:	PROJECT_LIBS = -lNetg -lboost_fsg -lexamg -lfcopyg -lstemg
ifndef WITHOUT_STLPORT
stldbg-shared:	PROJECT_LIBS = -lNetstlg -lboost_fsstlg -lexamstlg -lfcopystlg -lstemstlg
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

# -*- Makefile -*- Time-stamp: <07/02/21 15:30:59 ptr>

SRCROOT := ../../..
COMPILER_NAME := gcc
# ALL_TAGS := install-release-shared install-dbg-shared
# CoMT_DIR := ../../external/complement/explore

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

# DEFS += -DUNIT_TEST
INCLUDES += -I${CoMT_INCLUDE_DIR} -I${BOOST_INCLUDE_DIR} -I..

LDFLAGS += -L${INSTALL_LIB_DIR} -Wl,-rpath=${INSTALL_LIB_DIR}

release-shared: PROJECT_LIBS = -lxmt -lsockios -lstem -lboost_regex -lboost_test_utf -lboost_fs
dbg-shared:     PROJECT_LIBS = -lxmtg -lsockiosg -lstemg -lboost_regexg -lboost_test_utfg -lboost_fsg
stldbg-shared:  PROJECT_LIBS = -lxmtstlg -lsockiosstlg -lstemstlg -lboost_regexstlg -lboost_test_utfstlg -lboost_fsstlg

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

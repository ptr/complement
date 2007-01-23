# -*- makefile -*- Time-stamp: <06/06/27 02:25:13 ptr>

SRCROOT := ../../../../../../explore
COMPILER_NAME := gcc

include Makefile.inc
include ${SRCROOT}/Makefiles/top.mak

INCLUDES += -I${BOOST_INCLUDE_DIR}

LDFLAGS += -L${INSTALL_LIB_DIR} -Wl,-rpath=${INSTALL_LIB_DIR}:${STLPORT_LIB_DIR}

release-shared:	LDLIBS = -lboost_test_tem -lboost_fs
dbg-shared:	LDLIBS = -lboost_test_temg -lboost_fsg
ifndef WITHOUT_STLPORT
stldbg-shared:	LDLIBS = -lboost_test_temstlg -lboost_fsstlg
endif

check:	all
	for p in ${PRGNAME} ${PRGNAMES}; do \
	  ${OUTPUT_DIR}/$$p || exit 1; \
	  ${OUTPUT_DIR_DBG}/$$p || exit 1; \
	done
ifndef WITHOUT_STLPORT
	for p in ${PRGNAME} ${PRGNAMES}; do \
	  ${OUTPUT_DIR_STLDBG}/$$p || exit 1; \
	done
endif
	rm -rf ./xx fstream_test_?foo

check-release-shared:	release-shared
	for p in ${PRGNAME} ${PRGNAMES}; do \
	  ${OUTPUT_DIR}/$$p || exit 1; \
	done
	rm -rf ./xx fstream_test_?foo

check-dbg-shared:	dbg-shared
	for p in ${PRGNAME} ${PRGNAMES}; do \
	  ${OUTPUT_DIR_DBG}/$$p || exit 1; \
	done
	rm -rf ./xx fstream_test_?foo

ifndef WITHOUT_STLPORT
check-stldbg-shared:	stldbg-shared
	for p in ${PRGNAME} ${PRGNAMES}; do \
	  ${OUTPUT_DIR_STLDBG}/$$p || exit 1; \
	done
	rm -rf ./xx fstream_test_?foo
endif


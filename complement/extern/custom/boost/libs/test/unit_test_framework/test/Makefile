# -*- makefile -*- Time-stamp: <06/06/27 02:25:13 ptr>

SRCROOT := ../../../../../../../explore
COMPILER_NAME := gcc

include Makefile.inc
include ${SRCROOT}/Makefiles/top.mak

INCLUDES += -I${BOOST_INCLUDE_DIR}

release-shared:	LDFLAGS += -L../${OUTPUT_DIR} -Wl,-rpath=../${OUTPUT_DIR}:${STLPORT_LIB_DIR}
dbg-shared:	LDFLAGS += -L../${OUTPUT_DIR_DBG} -Wl,-rpath=../${OUTPUT_DIR_DBG}:${STLPORT_LIB_DIR}
ifndef WITHOUT_STLPORT
stldbg-shared:	LDFLAGS += -L../${OUTPUT_DIR_STLDBG} -Wl,-rpath=../${OUTPUT_DIR_STLDBG}:${STLPORT_LIB_DIR}
endif

release-shared:	LDLIBS = -lboost_test_utf
dbg-shared:	LDLIBS = -lboost_test_utfg
ifndef WITHOUT_STLPORT
stldbg-shared:	LDLIBS = -lboost_test_utfstlg
endif

TMPFILES = pattern.temp2 pattern.test

check:	all
	for p in ${PRGNAME} ${PRGNAMES}; do \
	  ${OUTPUT_DIR}/$$p || exit 1; \
	done
	rm -f ${TMPFILES}
	for p in ${PRGNAME} ${PRGNAMES}; do \
	  ${OUTPUT_DIR_DBG}/$$p || exit 1; \
	done
	rm -f ${TMPFILES}
ifndef WITHOUT_STLPORT
	for p in ${PRGNAME} ${PRGNAMES}; do \
	  ${OUTPUT_DIR_STLDBG}/$$p || exit 1; \
	done
	rm -f ${TMPFILES}
endif

check-release-shared:	release-shared
	for p in ${PRGNAME} ${PRGNAMES}; do \
	  ${OUTPUT_DIR}/$$p || exit 1; \
	done
	rm -f ${TMPFILES}

check-dbg-shared:	dbg-shared
	for p in ${PRGNAME} ${PRGNAMES}; do \
	  ${OUTPUT_DIR_DBG}/$$p || exit 1; \
	done
	rm -f ${TMPFILES}

ifndef WITHOUT_STLPORT
check-stldbg-shared:	stldbg-shared
	for p in ${PRGNAME} ${PRGNAMES}; do \
	  ${OUTPUT_DIR_STLDBG}/$$p || exit 1; \
	done
	rm -f ${TMPFILES}
endif


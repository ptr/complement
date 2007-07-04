# -*- makefile -*- Time-stamp: <07/01/25 00:49:53 ptr>

SRCROOT := ../../../../../../explore
COMPILER_NAME := gcc

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

INCLUDES += -I${BOOST_INCLUDE_DIR}

release-shared:	LDFLAGS += -L../${OUTPUT_DIR} -L../../test/test_exec_mon/${OUTPUT_DIR} -Wl,-rpath=../${OUTPUT_DIR}:../../test/test_exec_mon/${OUTPUT_DIR}:${STLPORT_LIB_DIR}
dbg-shared:	LDFLAGS += -L../${OUTPUT_DIR_DBG} -L../../test/test_exec_mon/${OUTPUT_DIR_DBG} -Wl,-rpath=../${OUTPUT_DIR_DBG}:../../test/test_exec_mon/${OUTPUT_DIR_DBG}:${STLPORT_LIB_DIR}
ifndef WITHOUT_STLPORT
stldbg-shared:	LDFLAGS += -L../${OUTPUT_DIR_STLDBG} -L../../test/test_exec_mon/${OUTPUT_DIR_STLDBG} -Wl,-rpath=../${OUTPUT_DIR_STLDBG}:../../test/test_exec_mon/${OUTPUT_DIR_STLDBG}:${STLPORT_LIB_DIR}
endif

release-shared:	LDLIBS = -lboost_test_tem -lboost_program_options
dbg-shared:	LDLIBS = -lboost_test_temg -lboost_program_optionsg
ifndef WITHOUT_STLPORT
stldbg-shared:	LDLIBS = -lboost_test_temstlg -lboost_program_optionsstlg
endif

check:	all
	@for p in ${PRGNAME} ${PRGNAMES}; do \
	  ${OUTPUT_DIR}/$$p || exit 1; \
	done
	@for p in ${PRGNAME} ${PRGNAMES}; do \
	  ${OUTPUT_DIR_DBG}/$$p || exit 1; \
	done
ifndef WITHOUT_STLPORT
	@for p in ${PRGNAME} ${PRGNAMES}; do \
	  ${OUTPUT_DIR_STLDBG}/$$p || exit 1; \
	done
endif

check-release-shared:	release-shared
	@for p in ${PRGNAME} ${PRGNAMES}; do \
	  ${OUTPUT_DIR}/$$p || exit 1; \
	done

check-dbg-shared:	dbg-shared
	@for p in ${PRGNAME} ${PRGNAMES}; do \
	  ${OUTPUT_DIR_DBG}/$$p || exit 1; \
	done

ifndef WITHOUT_STLPORT
check-stldbg-shared:	stldbg-shared
	@for p in ${PRGNAME} ${PRGNAMES}; do \
	  ${OUTPUT_DIR_STLDBG}/$$p || exit 1; \
	done
endif


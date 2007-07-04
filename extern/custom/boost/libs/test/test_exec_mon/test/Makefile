# -*- makefile -*- Time-stamp: <07/01/24 23:37:22 ptr>

SRCROOT := ../../../../../../../explore
COMPILER_NAME := gcc

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

INCLUDES += -I${BOOST_INCLUDE_DIR}

release-shared:	LDFLAGS += -L../${OUTPUT_DIR} -Wl,-rpath=../${OUTPUT_DIR}:${STLPORT_LIB_DIR}
dbg-shared:	LDFLAGS += -L../${OUTPUT_DIR_DBG} -Wl,-rpath=../${OUTPUT_DIR_DBG}:${STLPORT_LIB_DIR}
ifndef WITHOUT_STLPORT
stldbg-shared:	LDFLAGS += -L../${OUTPUT_DIR_STLDBG} -Wl,-rpath=../${OUTPUT_DIR_STLDBG}:${STLPORT_LIB_DIR}
endif

release-shared:	LDLIBS = -lboost_test_tem
dbg-shared:	LDLIBS = -lboost_test_temg
ifndef WITHOUT_STLPORT
stldbg-shared:	LDLIBS = -lboost_test_temstlg
endif

check:	all
	@for p in ${PRGNAME} ${PRGNAMES}; do \
	  if [ "$$p" != "boost_check_equal_str" ]; then \
	    ${OUTPUT_DIR}/$$p >/dev/null 2>&1; \
	    if [ $$? -eq 0 ]; then echo $$p; exit 1; fi \
	  else \
	    ${OUTPUT_DIR}/$$p; \
	  fi \
	done
	@for p in ${PRGNAME} ${PRGNAMES}; do \
	  if [ "$$p" != "boost_check_equal_str" ]; then \
	    ${OUTPUT_DIR_DBG}/$$p >/dev/null 2>&1; \
	    if [ $$? -eq 0 ]; then echo $$p; exit 1; fi \
	  else \
	    ${OUTPUT_DIR_DBG}/$$p; \
	  fi \
	done
ifndef WITHOUT_STLPORT
	@for p in ${PRGNAME} ${PRGNAMES}; do \
	  if [ "$$p" != "boost_check_equal_str" ]; then \
	    ${OUTPUT_DIR_STLDBG}/$$p >/dev/null 2>&1; \
	    if [ $$? -eq 0 ]; then echo $$p; exit 1; fi \
	  else \
	    ${OUTPUT_DIR_STLDBG}/$$p; \
	  fi \
	done
endif

check-release-shared:	release-shared
	@for p in ${PRGNAME} ${PRGNAMES}; do \
	  if [ "$$p" != "boost_check_equal_str" ]; then \
	    ${OUTPUT_DIR}/$$p >/dev/null 2>&1; \
	    if [ $$? -eq 0 ]; then echo $$p; exit 1; fi; \
	  else \
	    ${OUTPUT_DIR}/$$p; \
	  fi \
	done

check-dbg-shared:	dbg-shared
	@for p in ${PRGNAME} ${PRGNAMES}; do \
	  if [ "$$p" != "boost_check_equal_str" ]; then \
	    ${OUTPUT_DIR_DBG}/$$p >/dev/null 2>&1; \
	    if [ $$? -eq 0 ]; then echo $$p; exit 1; fi \
	  else \
	    ${OUTPUT_DIR_DBG}/$$p; \
	  fi \
	done

ifndef WITHOUT_STLPORT
check-stldbg-shared:	stldbg-shared
	@for p in ${PRGNAME} ${PRGNAMES}; do \
	  if [ "$$p" != "boost_check_equal_str" ]; then \
	    ${OUTPUT_DIR_STLDBG}/$$p >/dev/null 2>&1; \
	    if [ $$? -eq 0 ]; then echo $$p; exit 1; fi \
	  else \
	    ${OUTPUT_DIR_STLDBG}/$$p; \
	  fi \
	done
endif

# -*- makefile -*- Time-stamp: <07/01/24 23:48:37 ptr>

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

release-shared:	LDLIBS = -lboost_test_pem
dbg-shared:	LDLIBS = -lboost_test_pemg
ifndef WITHOUT_STLPORT
stldbg-shared:	LDLIBS = -lboost_test_pemstlg
endif

check:	all
	@for p in ${PRGNAME} ${PRGNAMES}; do \
	  s=`${OUTPUT_DIR}/$$p 2>&1 | grep 'errors detected'`; \
	  if [ "$$s" = "" ]; then echo $$p; exit 1; fi \
	done
	@for p in ${PRGNAME} ${PRGNAMES}; do \
	  s=`${OUTPUT_DIR_DBG}/$$p 2>&1 | grep 'errors detected'`; \
	  if [ "$$s" = "" ]; then echo $$p; exit 1; fi \
	done
ifndef WITHOUT_STLPORT
	@for p in ${PRGNAME} ${PRGNAMES}; do \
	  s=`${OUTPUT_DIR_STLDBG}/$$p 2>&1 | grep 'errors detected'`; \
	  if [ "$$s" = "" ]; then echo $$p; exit 1; fi \
	done
endif

check-release-shared:	release-shared
	@for p in ${PRGNAME} ${PRGNAMES}; do \
	  s=`${OUTPUT_DIR}/$$p 2>&1 | grep 'errors detected'`; \
	  if [ "$$s" = "" ]; then echo $$p; exit 1; fi \
	done

check-dbg-shared:	dbg-shared
	@for p in ${PRGNAME} ${PRGNAMES}; do \
	  s=`${OUTPUT_DIR_DBG}/$$p 2>&1 | grep 'errors detected'`; \
	  if [ "$$s" = "" ]; then echo $$p; exit 1; fi \
	done

ifndef WITHOUT_STLPORT
check-stldbg-shared:	stldbg-shared
	@for p in ${PRGNAME} ${PRGNAMES}; do \
	  s=`${OUTPUT_DIR_STLDBG}/$$p 2>&1 | grep 'errors detected'`; \
	  if [ "$$s" = "" ]; then echo $$p; exit 1; fi \
	done
endif


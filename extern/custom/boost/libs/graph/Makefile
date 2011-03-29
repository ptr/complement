# -*- makefile -*- Time-stamp: <06/06/27 02:25:13 ptr>

SRCROOT := ../../../../../explore

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

INCLUDES += -I${BOOST_INCLUDE_DIR}

check:	all
	${MAKE} -C test check

check-release-shared:	release-shared
	${MAKE} -C test check-release-shared

check-dbg-shared:	dbg-shared
	${MAKE} -C test check-dbg-shared

ifndef WITHOUT_STLPORT
check-stldbg-shared:	stldbg-shared
	${MAKE} -C test check-stldbg-shared
endif


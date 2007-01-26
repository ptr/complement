# -*- makefile -*- Time-stamp: <07/01/24 22:10:47 ptr>

SRCROOT := ../../../../../../explore
COMPILER_NAME := gcc

include Makefile.inc
include ${SRCROOT}/Makefiles/top.mak

INCLUDES += -I${BOOST_INCLUDE_DIR}

check:	all-shared
	$(MAKE) -C test check || exit 1

check-release-shared:	release-shared
	$(MAKE) -C test check-release-shared || exit 1

check-dbg-shared:	dbg-shared
	$(MAKE) -C test check-dbg-shared || exit 1

ifndef WITHOUT_STLPORT
check-stldbg-shared:	stldbg-shared
	$(MAKE) -C test check-stldbg-shared || exit 1
endif

depend::
	$(MAKE) -C test depend

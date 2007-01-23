# -*- Makefile -*- Time-stamp: <07/01/23 14:59:53 ptr>

SRCROOT := ../..
COMPILER_NAME := gcc

include Makefile.inc
include ${SRCROOT}/Makefiles/top.mak

INCLUDES += -I$(SRCROOT)/include

check:	install
	$(MAKE) -C ../../test/sockios
	(cd ../../test/sockios; ${OUTPUT_DIR}/sockios_ut) || exit 1
	(cd ../../test/sockios; ${OUTPUT_DIR_DBG}/sockios_ut) || exit 1
	(cd ../../test/sockios; ${OUTPUT_DIR_STLDBG}/sockios_ut) || exit 1

check-release:	install-release-shared
	$(MAKE) -C ../../test/sockios release-shared
	(cd ../../test/sockios; ${OUTPUT_DIR}/sockios_ut) || exit 1

check-dbg:	install-dbg-shared
	$(MAKE) -C ../../test/sockios dbg-shared
	(cd ../../test/sockios; ${OUTPUT_DIR_DBG}/sockios_ut) || exit 1

ifndef WITHOUT_STLPORT
check-stldbg:	install-stldbg-shared
	$(MAKE) -C ../../test/sockios stldbg-shared
	(cd ../../test/sockios; ${OUTPUT_DIR_STLDBG}/sockios_ut) || exit 1
endif

depend::
	$(MAKE) -C ../../test/sockios depend

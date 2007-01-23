# -*- Makefile -*- Time-stamp: <07/01/23 15:00:11 ptr>

SRCROOT := ../..

include Makefile.inc
include ${SRCROOT}/Makefiles/top.mak

INCLUDES += -I$(SRCROOT)/include

check:	install
	$(MAKE) -C ../../test/mt
	(cd ../../test/mt; ${OUTPUT_DIR}/mt_ut) || exit 1
	(cd ../../test/mt; ${OUTPUT_DIR_DBG}/mt_ut) || exit 1
	(cd ../../test/mt; ${OUTPUT_DIR_STLDBG}/mt_ut) || exit 1

check-release:	install-release-shared
	$(MAKE) -C ../../test/mt release-shared
	(cd ../../test/mt; ${OUTPUT_DIR}/mt_ut) || exit 1

check-dbg:	install-dbg-shared
	$(MAKE) -C ../../test/mt dbg-shared
	(cd ../../test/mt; ${OUTPUT_DIR_DBG}/mt_ut) || exit 1

ifndef WITHOUT_STLPORT
check-stldbg:	install-stldbg-shared
	$(MAKE) -C ../../test/mt stldbg-shared
	(cd ../../test/mt; ${OUTPUT_DIR_STLDBG}/mt_ut) || exit 1
endif

depend::
	$(MAKE) -C ../../test/mt depend

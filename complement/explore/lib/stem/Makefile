# -*- Makefile -*- Time-stamp: <07/08/03 09:12:25 ptr>

SRCROOT := ../..

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

INCLUDES += -I$(SRCROOT)/include

dbg-shared:	DEFS += -D__FIT_STEM_TRACE=1

ifndef WITHOUT_STLPORT
stldbg-shared:	DEFS += -D__FIT_STEM_TRACE=1
endif

check:	all-shared
	$(MAKE) -C ../../test/stem
	(cd ../../test/stem; ${OUTPUT_DIR}/stem_ut) || exit 1
	(cd ../../test/stem; ${OUTPUT_DIR_DBG}/stem_ut) || exit 1
ifndef WITHOUT_STLPORT
	(cd ../../test/stem; ${OUTPUT_DIR_STLDBG}/stem_ut) || exit 1
endif

check-release-shared:	release-shared
	$(MAKE) -C ../../test/stem release-shared
	(cd ../../test/stem; ${OUTPUT_DIR}/stem_ut) || exit 1

check-dbg-shared:	dbg-shared
	$(MAKE) -C ../../test/stem dbg-shared
	(cd ../../test/stem; ${OUTPUT_DIR_DBG}/stem_ut) || exit 1

ifndef WITHOUT_STLPORT
check-stldbg-shared:	stldbg-shared
	$(MAKE) -C ../../test/stem stldbg-shared
	(cd ../../test/stem; ${OUTPUT_DIR_STLDBG}/stem_ut) || exit 1
endif

depend::
	$(MAKE) -C ../../test/stem depend

# -*- Makefile -*- Time-stamp: <07/08/03 23:55:31 ptr>

SRCROOT := ../..

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

INCLUDES += -I$(SRCROOT)/include

dbg-shared:	DEFS += -D__FIT_STEM_TRACE=1

ifndef WITHOUT_STLPORT
stldbg-shared:	DEFS += -D__FIT_STEM_TRACE=1
endif

check:	all-shared
	$(MAKE) -C ut
	(cd ut; ${OUTPUT_DIR}/stem_ut) || exit 1
	(cd ut; ${OUTPUT_DIR_DBG}/stem_ut) || exit 1
ifndef WITHOUT_STLPORT
	(cd ut; ${OUTPUT_DIR_STLDBG}/stem_ut) || exit 1
endif

check-release-shared:	release-shared
	$(MAKE) -C ut release-shared
	(cd ut; ${OUTPUT_DIR}/stem_ut) || exit 1

check-dbg-shared:	dbg-shared
	$(MAKE) -C ut dbg-shared
	(cd ut; ${OUTPUT_DIR_DBG}/stem_ut) || exit 1

ifndef WITHOUT_STLPORT
check-stldbg-shared:	stldbg-shared
	$(MAKE) -C ut stldbg-shared
	(cd ut; ${OUTPUT_DIR_STLDBG}/stem_ut) || exit 1
endif

depend clean distclean mostlyclean maintainer-clean::
	${MAKE} -C ut $@

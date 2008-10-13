# -*- Makefile -*- Time-stamp: <07/08/03 23:56:04 ptr>

SRCROOT := ../..

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

INCLUDES += -I$(SRCROOT)/include
HEADERS_BASE = $(SRCROOT)/include/mt $(SRCROOT)/include/config $(SRCROOT)/include/misc

# LDLIBS += -ldl -lbfd
LDLIBS += -ldl -lbfd

check:	all-shared
	$(MAKE) -C ut all-shared
	(cd ut; ${OUTPUT_DIR}/mt_ut) || exit 1
	(cd ut; ${OUTPUT_DIR_DBG}/mt_ut) || exit 1
ifndef WITHOUT_STLPORT
	(cd ut; ${OUTPUT_DIR_STLDBG}/mt_ut) || exit 1
endif

check-release-shared:	release-shared
	$(MAKE) -C ut release-shared
	(cd ut; ${OUTPUT_DIR}/mt_ut) || exit 1

check-dbg-shared:	dbg-shared
	$(MAKE) -C ut dbg-shared
	(cd ut; ${OUTPUT_DIR_DBG}/mt_ut) || exit 1

ifndef WITHOUT_STLPORT
check-stldbg-shared:	stldbg-shared
	$(MAKE) -C ut stldbg-shared
	(cd ut; ${OUTPUT_DIR_STLDBG}/mt_ut) || exit 1
endif

depend clean distclean mostlyclean maintainer-clean::
	${MAKE} -C ut $@

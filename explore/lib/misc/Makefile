# -*- Makefile -*- Time-stamp: <08/12/04 16:03:02 ptr>

SRCROOT := ../..

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

INCLUDES += -I$(SRCROOT)/include

check:  all-shared
	$(MAKE) -C ut
	(cd ut; ${OUTPUT_DIR}/misc_sut) || exit 1
	(cd ut; ${OUTPUT_DIR_DBG}/misc_sut) || exit 1
ifndef WITHOUT_STLPORT
	(cd ut; ${OUTPUT_DIR_STLDBG}/misc_sut) || exit 1
endif

check-release-shared:   release-shared
	$(MAKE) -C ut release-shared
	(cd ut; ${OUTPUT_DIR}/misc_sut) || exit 1

check-dbg-shared:       dbg-shared
	$(MAKE) -C ut dbg-shared
	(cd ut; ${OUTPUT_DIR_DBG}/misc_sut) || exit 1

ifndef WITHOUT_STLPORT
check-stldbg-shared:    stldbg-shared
	$(MAKE) -C ut stldbg-shared
	(cd ut; ${OUTPUT_DIR_STLDBG}/misc_sut) || exit 1
endif

depend clean distclean mostlyclean maintainer-clean::
	${MAKE} -C ut $@


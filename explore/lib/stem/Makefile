# -*- Makefile -*- Time-stamp: <10/07/09 14:33:05 ptr>

SRCROOT := ../..

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

INCLUDES += -I$(SRCROOT)/include

dbg-shared:	DEFS += -D__FIT_STEM_TRACE=1

ifndef WITHOUT_STLPORT
stldbg-shared:	DEFS += -D__FIT_STEM_TRACE=1
endif

check:	all-shared
	$(MAKE) -C ut $@

check-release-shared:	release-shared
	$(MAKE) -C ut $@

check-dbg-shared:	dbg-shared
	$(MAKE) -C ut $@

ifndef WITHOUT_STLPORT
check-stldbg-shared:	stldbg-shared
	$(MAKE) -C ut $@
endif

depend clean distclean mostlyclean maintainer-clean::
	${MAKE} -C ut $@

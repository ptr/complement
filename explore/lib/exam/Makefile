# -*- Makefile -*- Time-stamp: <2011-01-18 00:01:05 ptr>

SRCROOT := ../..

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

INCLUDES += -I$(SRCROOT)/include
CXXFLAGS += -Wall

check:	all-shared
	$(MAKE) -C ./ut $@

check-release-shared:	release-shared
	$(MAKE) -C ./ut $@

check-dbg-shared:	dbg-shared
	$(MAKE) -C ./ut $@

ifndef WITHOUT_STLPORT
check-stldbg-shared:	stldbg-shared
	$(MAKE) -C ./ut $@
endif

depend clean distclean mostlyclean maintainer-clean::
	${MAKE} -C ut $@

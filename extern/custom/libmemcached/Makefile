# -*- makefile -*- Time-stamp: <09/01/29 11:02:18 ptr>

SRCROOT := ../../../explore
COMPILER_NAME := gcc

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

INCLUDES += -I${LIBMEMCACHED_SRC}/.. -I${LIBMEMCACHED_SRC}
DEFS += -DHAVE_UNISTD_H -DHAVE_STDINT_H -DHAVE_STRING_H

LDLIBS += 

check:	all-shared
	$(MAKE) -C test check || exit 1

check-release:	release-shared
	$(MAKE) -C test check-release-shared || exit 1

check-dbg:	dbg-shared
	$(MAKE) -C test check-dbg-shared || exit 1

ifndef WITHOUT_STLPORT
check-stldbg:	stldbg-shared
	$(MAKE) -C test check-stldbg-shared || exit 1
endif

depend::
	$(MAKE) -C test depend

clean::
	$(MAKE) -C test clean

distclean::
	$(MAKE) -C test distclean

mostlyclean::
	$(MAKE) -C test mostlyclean

maintainer-clean::
	$(MAKE) -C test maintainer-clean

# -*- makefile -*- Time-stamp: <09/01/29 10:26:04 ptr>

SRCROOT := ../../../explore
COMPILER_NAME := gcc

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

INCLUDES += -I${MEMCACHED_SRC}
DEFS += -DHAVE_UNISTD_H -DHAVE_STDINT_H -DHAVE_STRING_H -DUSE_THREADS -DVERSION=\"1.2.6-current\" -DPACKAGE=\"memcached\" -DENDIAN_LITTLE=1

LDLIBS += -levent

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

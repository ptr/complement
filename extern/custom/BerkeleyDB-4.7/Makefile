# -*- makefile -*- Time-stamp: <09/03/19 09:31:43 ptr>

SRCROOT ?= ../../../explore
COMPILER_NAME := gcc

# EXTRA_PRE = configure
# EXTRA_PRE_DBG = configure
# EXTRA_PRE_STLDBG = configure

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

INCLUDES += -Ibuild_unix -I${BERKELEYDB_SRC}
DEFS += -D_GNU_SOURCE -D_REENTRANT

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
	[ -d build_unix ] && rm -r build_unix
	$(MAKE) -C test maintainer-clean

configure:
	[ -d build_unix ] || mkdir build_unix
	cd build_unix && ../../../BerkeleyDB/dist/configure --enable-compat185 --enable-cxx

PHONY += configure

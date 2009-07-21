# -*- makefile -*- Time-stamp: <09/07/21 11:26:31 ptr>

SRCROOT := ../../../explore
COMPILER_NAME := gcc

# EXTRA_PRE = configure
# EXTRA_PRE_DBG = configure
# EXTRA_PRE_STLDBG = configure

include ${SRCROOT}/Makefiles/gmake/top.mak

BERKELEYDB_SRC ?= ${HOME}/labs/BerkeleyDB

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
	$(MAKE) -C db depend
	$(MAKE) -C db_cxx depend

clean::
	$(MAKE) -C db_cxx clean
	$(MAKE) -C db clean

distclean::
	$(MAKE) -C db_cxx distclean
	$(MAKE) -C db distclean

mostlyclean::
	$(MAKE) -C db_cxx mostlyclean
	$(MAKE) -C db mostlyclean

maintainer-clean::
	[ -d build_unix ] && rm -r build_unix
	$(MAKE) -C db_cxx maintainer-clean
	$(MAKE) -C db maintainer-clean

configure:
	[ -d build_unix ] || mkdir build_unix
	cd build_unix && { [ -z "${PREFIX}" ] && ${BERKELEYDB_SRC}/dist/configure --enable-compat185 --enable-cxx --disable-mutexsupport --disable-queue --disable-replication --disable-statistics --disable-verify || ${BERKELEYDB_SRC}/dist/configure --prefix=$(PREFIX) --enable-compat185 --enable-cxx --disable-mutexsupport --disable-queue --disable-replication --disable-statistics --disable-verify; }

PHONY += configure

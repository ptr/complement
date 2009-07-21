# -*- makefile -*- Time-stamp: <09/07/21 11:33:33 ptr>

SRCROOT ?= ../../../../explore
COMPILER_NAME := gcc

# EXTRA_PRE = configure
# EXTRA_PRE_DBG = configure
# EXTRA_PRE_STLDBG = configure

BERKELEYDB_SRC ?= ${HOME}/labs/BerkeleyDB

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

INCLUDES += -I../build_unix -I${BERKELEYDB_SRC}
DEFS += -D_GNU_SOURCE -D_REENTRANT

release-shared:	LDFLAGS += -L../db/${OUTPUT_DIR}

dbg-shared:	LDFLAGS += -L../db/${OUTPUT_DIR_DBG}

ifndef WITHOUT_STLPORT
stldbg-shared:	LDFLAGS += -L../db/${OUTPUT_DIR_DBG}
endif

release-shared:	LDLIBS += -ldb

dbg-shared:	LDLIBS += -ldbg

ifndef WITHOUT_STLPORT
stldbg-shared:	LDLIBS += -ldbg
endif

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

#depend::
#	$(MAKE) -C test depend

#clean::
#	$(MAKE) -C test clean

#distclean::
#	$(MAKE) -C test distclean

#mostlyclean::
#	$(MAKE) -C test mostlyclean

maintainer-clean::
	[ -d build_unix ] && rm -r build_unix
#	$(MAKE) -C test maintainer-clean

configure:
	[ -d ../build_unix ] || mkdir ../build_unix
	cd ../build_unix && { [ -z "${PREFIX}" ] && ${BERKELEYDB_SRC}/dist/configure --enable-compat185 --enable-cxx --disable-mutexsupport --disable-queue --disable-replication --disable-statistics --disable-verify || ${BERKELEYDB_SRC}/dist/configure --prefix=$(PREFIX) --enable-compat185 --enable-cxx --disable-mutexsupport --disable-queue --disable-replication --disable-statistics --disable-verify; }

PHONY += configure

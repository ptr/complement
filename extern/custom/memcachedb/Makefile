# -*- makefile -*- Time-stamp: <09/03/19 09:27:02 ptr>

SRCROOT := ../../../explore
COMPILER_NAME := gcc

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

INCLUDES += -I${MEMCACHEDB_SRC}

ifndef SYSLIBEVENT
INCLUDES += -I../libevent -I../../libevent/libevent
release-shared: LDFLAGS += -L../libevent/${OUTPUT_DIR}
release-shared: LDLIBS += -levent
dbg-shared:	LDFLAGS += -L../libevent/${OUTPUT_DIR_DBG}
dbg-shared:	LDLIBS += -leventg
ifndef WITHOUT_STLPORT
stldbg-shared:	LDFLAGS += -L../libevent/${OUTPUT_DIR_DBG}
stldbg-shared:	LDLIBS += -leventg
endif
else
LDLIBS += -levent
endif

ifndef SYSBDB
INCLUDES += -I../BerkeleyDB/build_unix
release-shared: LDFLAGS += -L../BerkeleyDB/${OUTPUT_DIR}
release-shared: LDLIBS += -ldb
dbg-shared: LDFLAGS += -L../BerkeleyDB/${OUTPUT_DIR_DBG}
dbg-shared: LDLIBS += -ldbg
ifndef WITHOUT_STLPORT
stldbg-shared: LDFLAGS += -L../BerkeleyDB/${OUTPUT_DIR_DBG}
stldbg-shared: LDLIBS += -ldbg
endif
else
LDLIBS += -ldb
endif

DEFS += -DHAVE_UNISTD_H -DHAVE_STDINT_H -DHAVE_STRING_H -DHAVE_MALLOC_H -DUSE_THREADS -DVERSION=\"1.2.1-dev\" -DPACKAGE=\"memcachedb\" -DENDIAN_LITTLE=1

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

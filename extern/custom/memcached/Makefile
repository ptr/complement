# -*- makefile -*- Time-stamp: <09/03/19 14:48:39 yeti>

SRCROOT := ../../../explore
COMPILER_NAME := gcc

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

INCLUDES += -I${MEMCACHED_SRC}
DEFS += -DNDEBUG -DHAVE_UNISTD_H -DHAVE_STDINT_H -DHAVE_STRING_H -DUSE_THREADS -DVERSION=\"1.3.2-dev\" -DPACKAGE=\"memcached\" -DENDIAN_LITTLE=1

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

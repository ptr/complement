# -*- makefile -*- Time-stamp: <09/03/19 14:48:39 yeti>

SRCROOT := ../../../explore
COMPILER_NAME := gcc

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

INCLUDES += -I${LUA_SRC}
DEFS += -DLUA_USE_LINUX
#DEFS += -DLUA_USE_POSIX -DLUA_USE_DLOPEN

LDFLAGS += -L../liblua/${OUTPUT_DIR}
LDLIBS += -ldl --static -llua --shared -lm

check:	all-shared
	$(MAKE) -C test check || exit 1

check-release:	release-shared
	$(MAKE) -C test check-release-shared || exit 1

check-dbg:	dbg-shared
	$(MAKE) -C test check-dbg-shared || exit 1

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

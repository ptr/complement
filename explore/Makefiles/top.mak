# Time-stamp: <03/07/09 18:23:32 ptr>
# $Id$

RULESBASE ?= $(SRCROOT)/Makefiles

ALL_TAGS ?= release-shared	dbg-shared	stldbg-shared

all:	$(ALL_TAGS)

# define what make clone we use
include ${RULESBASE}/make.mak
# identify OS and build date
include ${RULESBASE}/sysid-$(USE_MAKE).mak
# OS-specific definitions, like ar, ln, install, etc.
include ${RULESBASE}/sys-$(OSNAME).mak
# rules to make dirs for targets
include ${RULESBASE}/targetdirs.mak

# derive common targets (*.o, *.d),
# build rules (including output catalogs)
include ${RULESBASE}/targets-$(USE_MAKE).mak
# dependency
include ${RULESBASE}/depend-$(COMPILER_NAME).mak

# target is library, rules for library
ifdef LIBNAME
include ${RULESBASE}/lib/top.mak
endif

# general clean

include ${RULESBASE}/clean.mak

# Time-stamp: <03/07/10 15:20:25 ptr>
# $Id$

.SUFFIXES:
.SCCS_GET:
.RCS_GET:

PHONY ?=

.PHONY: $(PHONY)

RULESBASE ?= $(SRCROOT)/Makefiles

ALL_TAGS ?= release-shared	dbg-shared	stldbg-shared

all:	dirs $(ALL_TAGS)

# define what make clone we use
include ${RULESBASE}/make.mak
# identify OS and build date
include ${RULESBASE}/sysid-$(USE_MAKE).mak
# OS-specific definitions, like ar, ln, install, etc.
include ${RULESBASE}/sys-$(OSNAME).mak
# rules to make dirs for targets
include ${RULESBASE}/targetdirs.mak
# extern libraries
include ${RULESBASE}/extern-$(OSNAME).mak

# derive common targets (*.o, *.d),
# build rules (including output catalogs)
include ${RULESBASE}/targets-$(USE_MAKE).mak
# dependency
include ${RULESBASE}/depend-$(COMPILER_NAME).mak

# general clean
include ${RULESBASE}/clean.mak

# if target is library, rules for library
ifdef LIBNAME
include ${RULESBASE}/lib/top.mak
endif

# if target is program, rules for executable
ifdef PRGNAME
include ${RULESBASE}/app/top.mak
endif

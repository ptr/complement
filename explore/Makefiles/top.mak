# Time-stamp: <03/07/06 23:13:45 ptr>
# $Id$

include ${BASEDIR}/Makefiles/make.mak
include ${BASEDIR}/Makefiles/sysid-$(USE_MAKE).mak
include ${BASEDIR}/Makefiles/targetdirs.mak

DIRS_UNIQUE_SRC := $(dir $(SRC_CPP) $(SRC_CC) $(SRC_C) )
DIRS_UNIQUE_SRC := $(sort $(DIRS_UNIQUE_SRC) )
include ${BASEDIR}/Makefiles/dirsrc.mak
include ${BASEDIR}/Makefiles/sys-$(OSNAME).mak
include ${BASEDIR}/Makefiles/compiler-$(COMPILER_NAME).mak
include ${BASEDIR}/Makefiles/clean.mak

-include .make.depend

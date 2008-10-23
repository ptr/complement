# -*- makefile -*- Time-stamp: <03/06/05 18:35:11 ptr>
# $Id$

BASEDIR := $(shell xtmp=`pwd`; xtmp=`dirname $$xtmp`; dirname $$xtmp)
COMPILER_NAME = mwccnlm

include ./Makefile.inc

all:	release-shared

all-shared:	release-shared dbg-shared stldbg-shared

all-debug:	dbg-shared stldbg-shared

all-release:	release-shared

include ${BASEDIR}/Makefiles/app/Makefile.inc

INCLUDES += -I$(STLPORT_INCLUDE_DIR) -I../../include -I../..

BUILD_SYSTEM := $(shell echo "`uname -n` `uname -s` `uname -r` `uname -v` `uname -p`")
BUILD_DATE := $(shell echo "`date +'%Y/%m/%d %T %Z'`")

LDLIBS = e:/Novell/ndk/nwsdk/imports/clibpre.o

release-shared:	LDFLAGS += -L${STLPORT_LIB_DIR} -L${INSTALL_LIB_DIR}
stldbg-shared:	LDFLAGS += -L${STLPORT_LIB_DIR} -L${INSTALL_LIB_DIR_STLDBG}
dbg-shared:	LDFLAGS += -L${STLPORT_LIB_DIR} -L${INSTALL_LIB_DIR_DBG}
release-static:	LDFLAGS += -L${STLPORT_LIB_DIR} -L${INSTALL_LIB_DIR}
stldbg-static:	LDFLAGS += -L${STLPORT_LIB_DIR} -L${INSTALL_LIB_DIR_STLDBG}
dbg-static:	LDFLAGS += -L${STLPORT_LIB_DIR} -L${INSTALL_LIB_DIR_DBG}
SYSLIBS = 

dbg-shared : LDLIBS += \
					${SYSLIBS}

dbg-static : LDLIBS += \
					${SYSLIBS}


stldbg-shared : LDLIBS += \
					${SYSLIBS}

release-shared : LDLIBS += \
					${SYSLIBS}


FORCE:


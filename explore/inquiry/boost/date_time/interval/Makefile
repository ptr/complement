# Time-stamp: <03/06/22 16:44:39 ptr>
# $Id$

BASEDIR := $(shell xtmp=`pwd`; xtmp=`dirname $$xtmp`; xtmp=`dirname $$xtmp`; xtmp=`dirname $$xtmp`; dirname $$xtmp)

COMPILER_NAME = gcc
include ./Makefile.inc

all:	release-shared

all-shared:	release-shared dbg-shared stldbg-shared

all-debug:	dbg-shared stldbg-shared

all-release:	release-shared

include ${BASEDIR}/Makefiles/app/Makefile.inc

INCLUDES += -I$(STLPORT_INCLUDE_DIR) -I${BASEDIR}/include -I${BOOST_INCLUDE_DIR}

BUILD_SYSTEM := $(shell echo "`uname -n` `uname -s` `uname -r` `uname -v` `uname -p`")
BUILD_DATE := $(shell echo "`date +'%Y/%m/%d %T %Z'`")

ifeq ($(OS_VER),SunOS)
LDLIBS = 
else
LDLIBS = 
endif

LDLIBS += -lrt -lnsl

ifeq ($(OS_VER),SunOS)
dbg-shared:	LDFLAGS += -L${STLPORT_LIB_DIR} -L${BASEDIR}/build/lib
stldbg-shared:	LDFLAGS += -L${STLPORT_LIB_DIR} -L${BASEDIR}/build/lib
release-shared:	LDFLAGS += -L${STLPORT_LIB_DIR} -L${BASEDIR}/build/lib
endif

ifeq ($(OS_VER),Linux)
release-shared:	LDFLAGS += -L${STLPORT_LIB_DIR} -L${BASEDIR}/build/lib
stldbg-shared:	LDFLAGS += -L${STLPORT_LIB_DIR} -L${BASEDIR}/build/lib
dbg-shared:	LDFLAGS += -L${STLPORT_LIB_DIR} -L${BASEDIR}/build/lib
endif

dbg-shared : LDLIBS += \
                    -lstlport_gcc

stldbg-shared : LDLIBS += \
                    -lstlport_gcc_stldebug

release-shared : LDLIBS += \
                    -lstlport_gcc

FORCE:

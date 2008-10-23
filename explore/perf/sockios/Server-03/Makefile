# -*- makefile -*- Time-stamp: <02/11/20 09:37:29 ptr>
# $Id$

BASEDIR := $(shell xtmp=`pwd`; xtmp=`dirname $$xtmp`; xtmp=`dirname $$xtmp`; dirname $$xtmp)
COMPILER_NAME = gcc

include ./Makefile.inc

all:	release-shared

all-shared:	release-shared dbg-shared stldbg-shared

all-debug:	dbg-shared stldbg-shared

all-release:	release-shared

include ${BASEDIR}/Makefiles/app/Makefile.inc

INCLUDES += -I$(STLPORT_INCLUDE_DIR)

ifeq ($(OS_VER),SunOS)
LDLIBS = 
else
LDLIBS = 
endif

LDLIBS += -lrt -lnsl

ifeq ($(OS_VER),SunOS)
dbg-shared:	LDFLAGS += -L${INSTALL_LIB_DIR_DBG}
stldbg-shared:	LDFLAGS += -L${INSTALL_LIB_DIR_STLDBG}
release-shared:	LDFLAGS += -L${INSTALL_LIB_DIR}
SYSLIBS = -lnsl -lsocket
endif

ifeq ($(OS_VER),Linux)
release-shared:	LDFLAGS += -L${STLPORT_LIB_DIR} -L${INSTALL_LIB_DIR}
stldbg-shared:	LDFLAGS += -L${STLPORT_LIB_DIR} -L${INSTALL_LIB_DIR_STLDBG}
dbg-shared:	LDFLAGS += -L${STLPORT_LIB_DIR} -L${INSTALL_LIB_DIR_DBG}
SYSLIBS =
endif

dbg-shared : LDLIBS += \
                    -lstlport_gcc \
		    -lxmt_gcc-g \
		    -lsockios_gcc-g -lmisc_gcc-g ${SYSLIBS}

stldbg-shared : LDLIBS += \
                    -lstlport_gcc_stldebug \
		    -lxmt_gcc_stl-g \
		    -lsockios_gcc_stl-g -lmisc_gcc_stl-g ${SYSLIBS}

release-shared : LDLIBS += \
                    -lstlport_gcc \
		    -lxmt_gcc \
		    -lsockios_gcc -lmisc_gcc ${SYSLIBS}

FORCE:


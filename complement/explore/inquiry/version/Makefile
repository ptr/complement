# -*- makefile -*- Time-stamp: <03/04/23 14:30:07 ptr>
# $Id$

BASEDIR := $(shell xtmp=`pwd`; xtmp=`dirname $$xtmp`; xtmp=`dirname $$xtmp`; dirname $$xtmp)
COMPILER_NAME = gcc

include ./Makefile.inc

all:	release-shared

all-shared:	release-shared dbg-shared stldbg-shared

all-debug:	dbg-shared stldbg-shared

all-release:	release-shared

include ${BASEDIR}/Makefiles/app/Makefile.inc

INCLUDES += -I$(STLPORT_INCLUDE_DIR) -I../../include -I../..

BUILD_SYSTEM := $(shell echo "`uname -n` `uname -s` `uname -r` `uname -v` `uname -p`")
BUILD_DATE := $(shell echo "`date +'%Y/%m/%d %T %Z'`")

ifeq ($(OS_VER),SunOS)
LDLIBS = 
else
LDLIBS = 
endif

LDLIBS += 

ifeq ($(OS_VER),SunOS)
dbg-shared:	LDFLAGS += -L${INSTALL_LIB_DIR_DBG}
stldbg-shared:	LDFLAGS += -L${INSTALL_LIB_DIR_STLDBG}
release-shared:	LDFLAGS += -L${INSTALL_LIB_DIR}
SYSLIBS =
endif

ifeq ($(OS_VER),Linux)
release-shared:	LDFLAGS += -L${STLPORT_LIB_DIR} -L${INSTALL_LIB_DIR}
stldbg-shared:	LDFLAGS += -L${STLPORT_LIB_DIR} -L${INSTALL_LIB_DIR_STLDBG}
dbg-shared:	LDFLAGS += -L${STLPORT_LIB_DIR} -L${INSTALL_LIB_DIR_DBG}
SYSLIBS = -ldl -lrt
endif

dbg-shared : LDLIBS += \
                    -lstlport_gcc \
					-lxmt_gcc-g \
					-ltest_runner_gcc-g \
					-ltesting_gcc-g \
					-lredir_gcc-g \
					-lerror_test_gcc-g \
					-lstd_gcc-g \
					-lerror_gcc-g \
					-Wl,-Bstatic -Wl,--start-group \
					-lklcsc_gcc-g \
					-lklcsa_gcc-g \
					-lgsoap_gcc-g \
					-Wl,--end-group -Wl,-Bdynamic \
					-lboost_fs_gcc-g \
					${SYSLIBS}


stldbg-shared : LDLIBS += \
                    -lstlport_gcc_stldebug \
					-lxmt_gcc_stl-g \
					-ltest_runner_gcc_stl-g \
					-lredir_gcc_stl-g \
					-lpar_gcc_stl-g \
					-Wl,-Bstatic -Wl,--start-group \
					-lgsoap_gcc_stl-g \
					-Wl,--end-group -Wl,-Bdynamic \
					${SYSLIBS}

release-shared : LDLIBS += \
                    -lstlport_gcc \
					-lxmt_gcc \
					-ltest_runner_gcc \
					-lredir_gcc \
					-lpar_gcc \
					-Wl,-Bstatic -Wl,--start-group \
					-lgsoap_gcc \
					-Wl,--end-group -Wl,-Bdynamic \
					${SYSLIBS}


FORCE:


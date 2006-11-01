# -*- Makefile -*- Time-stamp: <03/07/09 18:08:47 ptr>
# $Id$

SRCROOT := ../../..
COMPILER_NAME := gcc

include Makefile.inc
include ${SRCROOT}/Makefiles/top.mak


INCLUDES += -I$(SRCROOT)/include -I$(STLPORT_INCLUDE_DIR)

release-shared:	LDSEARCH = -L${STLPORT_LIB_DIR} -L${CoMT_LIB_DIR}
stldbg-shared:	LDSEARCH = -L${STLPORT_LIB_DIR} -L${CoMT_LIB_DIR_STLDBG}
dbg-shared:	LDSEARCH = -L${STLPORT_LIB_DIR} -L${CoMT_LIB_DIR_DBG}

ifeq ($(OSNAME),sunos)
release-shared : LDLIBS = -lstlport_gcc -lxmt_gcc -lmisc_gcc -lnsl -lsocket
stldbg-shared  : LDLIBS = -lstlport_gcc_stldebug -lxmt_gcc_stl-g -lmisc_gcc_stl-g -lnsl -lsocket
dbg-shared     : LDLIBS = -lstlport_gcc -lxmt_gcc-g -lmisc_gcc-g -lnsl -lsocket
else
release-shared : LDLIBS = -lstlport_gcc -lxmt_gcc -lmisc_gcc -lsockios_gcc
stldbg-shared  : LDLIBS = -lstlport_gcc_stldebug -lxmt_gcc_stl-g -lmisc_gcc_stl-g -lsockios_gcc_stl-g
dbg-shared     : LDLIBS = -lstlport_gcc -lxmt_gcc-g -lmisc_gcc-g -lsockios_gcc-g
endif


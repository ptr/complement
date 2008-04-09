# -*- Makefile -*- Time-stamp: <04/05/06 14:30:08 ptr>
# $Id: Makefile,v 1.1 2004/06/16 14:24:07 ptr Exp $

#SRCROOT := ../../../../extern/complement
SRCROOT := ../../../../../../island/workshop/explore
COMPILER_NAME := gcc

include Makefile.inc
include ${SRCROOT}/Makefiles/top.mak


DEFS += -D_NO_TEST
INCLUDES += -I$(SRCROOT)/include -I$(STLPORT_INCLUDE_DIR) -I$(BOOST_INCLUDE_DIR)

release-shared:	LDSEARCH = -L${STLPORT_LIB_DIR} -L${CoMT_LIB_DIR} -Wl,-rpath=${STLPORT_LIB_DIR}
stldbg-shared:	LDSEARCH = -L${STLPORT_LIB_DIR} -L${CoMT_LIB_DIR_STLDBG} -Wl,-rpath=${STLPORT_LIB_DIR}
dbg-shared:	LDSEARCH = -L${STLPORT_LIB_DIR} -L${CoMT_LIB_DIR_DBG} -Wl,-rpath=${STLPORT_LIB_DIR}

ifeq ($(OSNAME),sunos)
release-shared : LDLIBS = -lstlport_gcc -lrt
stldbg-shared  : LDLIBS = -lstlport_gcc_stldebug -lrt
dbg-shared     : LDLIBS = -lstlport_gcc_debug -lrt
else
release-shared : LDLIBS = -lstlport_gcc -lxmt_gcc -lsockios_gcc -lmisc_gcc -lboost_regex_gcc
stldbg-shared  : LDLIBS = -lstlport_gcc_stldebug -lxmt_gcc_stl-g -lsockios_gcc_stl-g -lmisc_gcc_stl-g -lboost_regex_gcc_stl-g
dbg-shared     : LDLIBS = -lstlport_gcc_debug -lxmt_gcc-g -lsockios_gcc-g -lmisc_gcc-g -lboost_regex_gcc-g
endif


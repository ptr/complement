# -*- makefile -*- Time-stamp: <03/03/31 15:18:29 ptr>
# $Id$

SRCROOT := ../../../../../../explore
COMPILER_NAME := gcc

include Makefile.inc
include ${SRCROOT}/Makefiles/top.mak

INCLUDES += -I$(STLPORT_INCLUDE_DIR) -I${BOOST_INCLUDE_DIR}

ifeq ($(OSNAME),linux)
release-shared: LDSEARCH = -L${STLPORT_LIB_DIR}
stldbg-shared:  LDSEARCH = -L${STLPORT_LIB_DIR}
dbg-shared:     LDSEARCH = -L${STLPORT_LIB_DIR}
endif

ifeq ($(OSNAME),sunos)
release-shared : LDLIBS = -lstlport_gcc -lrt
stldbg-shared  : LDLIBS = -lstlport_gcc_stldebug -lrt
dbg-shared     : LDLIBS = -lstlport_gcc -lrt
else
release-shared : LDLIBS = -lstlport_gcc
stldbg-shared  : LDLIBS = -lstlport_gcc_stldebug
dbg-shared     : LDLIBS = -lstlport_gcc
endif

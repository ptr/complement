# -*- Makefile -*- Time-stamp: <03/09/25 12:02:31 ptr>
# $Id$

SRCROOT := ../..
COMPILER_NAME := icc

include Makefile.inc
include ${SRCROOT}/Makefiles/top.mak

# DEFS += -D_LIBC=1 -D__USE_UNIX98=1 -D_USE_UNIX98=1 -D_PTHREADS=1
INCLUDES += -I$(SRCROOT)/include -I$(STLPORT_INCLUDE_DIR)

ifeq ($(OSNAME),linux)
release-shared:	LDSEARCH = -L${STLPORT_LIB_DIR}
stldbg-shared:	LDSEARCH = -L${STLPORT_LIB_DIR}
dbg-shared:	LDSEARCH = -L${STLPORT_LIB_DIR}
endif

ifeq ($(OSNAME),linux)
release-shared : LDLIBS = -lpthread -lstlport_icc
stldbg-shared  : LDLIBS = -lpthread -lstlport_icc_stldebug
dbg-shared     : LDLIBS = -lpthread -lstlport_icc_debug
endif

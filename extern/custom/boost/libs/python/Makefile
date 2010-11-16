# -*- makefile -*- Time-stamp: <09/10/05 23:11:53 ptr>

SRCROOT := ../../../../../explore
COMPILER_NAME := gcc

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

PYTHON_VERSION ?= 2.6

INCLUDES += -I${BOOST_INCLUDE_DIR} -I/usr/include/python${PYTHON_VERSION}

ifndef WITHOUT_STLPORT

release-shared: LDSEARCH = -L${STLPORT_LIB_DIR}
stldbg-shared:  LDSEARCH = -L${STLPORT_LIB_DIR}
dbg-shared:     LDSEARCH = -L${STLPORT_LIB_DIR}

ifeq ($(OSNAME),sunos)
release-shared : LDLIBS = -lstlport -lrt
stldbg-shared  : LDLIBS = -lstlportstlg -lrt
dbg-shared     : LDLIBS = -lstlportg -lrt
else
release-shared : LDLIBS = -lstlport
stldbg-shared  : LDLIBS = -lstlportstlg
dbg-shared     : LDLIBS = -lstlportg
endif

else

ifeq ($(OSNAME),sunos)
release-shared : LDLIBS = -lrt
stldbg-shared  : LDLIBS = -lrt
dbg-shared     : LDLIBS = -lrt
endif

endif

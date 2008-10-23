# -*- Makefile -*- Time-stamp: <07/08/08 22:18:48 ptr>

SRCROOT := ../../../..
COMPILER_NAME := gcc
# ALL_TAGS := install-release-shared install-dbg-shared
# CoMT_DIR := ../../external/complement/explore

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

# DEFS += -DUNIT_TEST
INCLUDES += -I${CoMT_INCLUDE_DIR}

release-shared: PROJECT_LIBS = -lxmt -lsockios -lstem -ljanus
dbg-shared:     PROJECT_LIBS = -lxmtg -lsockiosg -lstemg -ljanusg
ifndef WITHOUT_STLPORT
stldbg-shared:  PROJECT_LIBS = -lxmtstlg -lsockiosstlg -lstemstlg -ljanusstlg
endif

dbg-shared:    DEFS += -D__FIT_VS_TRACE
ifndef WITHOUT_STLPORT
stldbg-shared: DEFS += -D__FIT_VS_TRACE
endif

LDLIBS = -L${CoMT_LIB_DIR} -Wl,-rpath=${CoMT_LIB_DIR}:${STLPORT_LIB_DIR} ${PROJECT_LIBS}


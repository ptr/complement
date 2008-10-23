# -*- Makefile -*- Time-stamp: <07/10/15 20:16:57 ptr>

SRCROOT := ../../../..
COMPILER_NAME := gcc
# ALL_TAGS := install-release-shared install-dbg-shared
# CoMT_DIR := ../../external/complement/explore

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

# DEFS += -DUNIT_TEST
INCLUDES += -I${CoMT_INCLUDE_DIR}

release-shared: PROJECT_LIBS = -lxmt -lsockios -lstem
dbg-shared:     PROJECT_LIBS = -lxmtg -lsockiosg -lstemg
ifndef WITHOUT_STLPORT
stldbg-shared:  PROJECT_LIBS = -lxmtstlg -lsockiosstlg -lstemstlg
endif

LDLIBS = -L${CoMT_LIB_DIR} -Wl,-rpath=${CoMT_LIB_DIR}:${STLPORT_LIB_DIR} ${PROJECT_LIBS}

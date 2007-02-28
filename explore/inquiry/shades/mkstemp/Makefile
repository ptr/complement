# -*- Makefile -*- Time-stamp: <06/11/13 23:03:45 ptr>

SRCROOT := ../../..

include Makefile.inc
include ${SRCROOT}/Makefiles/top.mak

ifndef WITHOUT_STLPORT
LDFLAGS += -Wl,-rpath=${STLPORT_LIB_DIR}
endif

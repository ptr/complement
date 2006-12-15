# -*- Makefile -*- Time-stamp: <06/12/15 13:47:56 ptr>

SRCROOT := ../../..

include Makefile.inc
include ${SRCROOT}/Makefiles/top.mak

INCLUDES += -I${CoMT_INCLUDE_DIR}

release-shared : LDFLAGS += -Wl,-rpath=${STLPORT_LIB_DIR}:${CoMT_LIB_DIR}
dbg-shared : LDFLAGS += -Wl,-rpath=${STLPORT_LIB_DIR}:${CoMT_LIB_DIR}
stldbg-shared : LDFLAGS += -Wl,-rpath=${STLPORT_LIB_DIR}:${CoMT_LIB_DIR}

release-shared:	LDLIBS = -L${CoMT_LIB_DIR} -lxmt
dbg-shared:	LDLIBS = -L${CoMT_LIB_DIR} -lxmtg
stldbg-shared:	LDLIBS = -L${CoMT_LIB_DIR} -lxmtstlg

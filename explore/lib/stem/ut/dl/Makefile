# -*- Makefile -*-

SRCROOT := ../../../..

DBG_SUFFIX :=
STLDBG_SUFFIX :=
INSTALL_TAGS :=

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak
LIBSTEM_DIR = ${CoMT_DIR}/lib/stem

INCLUDES += -I$(SRCROOT)/include

#LDFLAGS = -L${STLPORT_LIB_DIR} -L${CoMT_LIB_DIR}
release-shared:	LDFLAGS += -L${LIBSTEM_DIR}/${OUTPUT_DIR}
dbg-shared:	LDFLAGS += -L${LIBSTEM_DIR}/${OUTPUT_DIR_DBG}
ifndef WITHOUT_STLPORT
stldbg-shared:	LDFLAGS += -L${LIBSTEM_DIR}/${OUTPUT_DIR_STLDBG}
endif

#release-shared : LDLIBS = -lxmt -lsockios -lstem -lboost_test_utf
release-shared:	LDLIBS = -lstem
#stldbg-shared  : LDLIBS = -lxmtstlg -lsockiosstlg -lstemstlg -lboost_test_utfstlg
#dbg-shared     : LDLIBS = -lxmtg -lsockiosg -lstemg -lboost_test_utfg
dbg-shared:	LDLIBS_DBG = -lstemg

ifndef WITHOUT_STLPORT
stldbg-shared:	LDLIBS_STLDBG = -lstemstlg
endif

#LDFLAGS += -Wl,-rpath=${STLPORT_LIB_DIR}:${CoMT_LIB_DIR}

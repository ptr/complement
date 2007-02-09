# -*- Makefile -*- Time-stamp: <07/02/07 12:13:04 ptr>

SRCROOT := ../../..
COMPILER_NAME := gcc

DBG_SUFFIX :=
STLDBG_SUFFIX :=
INSTALL_TAGS :=

include Makefile.inc
include ${SRCROOT}/Makefiles/top.mak
LIBSTEM_DIR = ${CoMT_DIR}/lib/stem

INCLUDES += -I$(SRCROOT)/include -I$(BOOST_INCLUDE_DIR)

#LDSEARCH = -L${STLPORT_LIB_DIR} -L${CoMT_LIB_DIR}
release-shared:	LDSEARCH += -L${LIBSTEM_DIR}/${OUTPUT_DIR}
stldbg-shared:	LDSEARCH += -L${LIBSTEM_DIR}/${OUTPUT_DIR_STLDBG}
dbg-shared:	LDSEARCH += -L${LIBSTEM_DIR}/${OUTPUT_DIR_DBG}

#release-shared : LDLIBS = -lxmt -lsockios -lstem -lboost_test_utf
release-shared:	LDLIBS = -lstem
#stldbg-shared  : LDLIBS = -lxmtstlg -lsockiosstlg -lstemstlg -lboost_test_utfstlg
stldbg-shared:	LDLIBS = -lstemstlg
#dbg-shared     : LDLIBS = -lxmtg -lsockiosg -lstemg -lboost_test_utfg
dbg-shared:	LDLIBS = -lstemg

#LDFLAGS += -Wl,-rpath=${STLPORT_LIB_DIR}:${CoMT_LIB_DIR}

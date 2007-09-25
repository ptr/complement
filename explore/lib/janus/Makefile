# -*- Makefile -*- Time-stamp: <07/02/07 12:28:46 ptr>

SRCROOT := ../..
COMPILER_NAME := gcc

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

INCLUDES += -I$(SRCROOT)/include

LIBMT_DIR = ${CoMT_DIR}/lib/mt
LIBSOCK_DIR = ${CoMT_DIR}/lib/sockios
LIBSTEM_DIR = ${CoMT_DIR}/lib/stem

LDSEARCH += -L${CoMT_LIB_DIR} -Wl,--rpath=${CoMT_LIB_DIR}

release-shared : LDLIBS = -lxmt -lsockios -lstem 
ifndef WITHOUT_STLPORT
stldbg-shared  : LDLIBS = -lxmtstlg -lsockiosstlg -lstemstlg
endif
dbg-shared     : LDLIBS = -lxmtg -lsockiosg -lstemg

check:	all-shared
	$(MAKE) -C ut
	(cd ut; ${OUTPUT_DIR}/ut_vtime) || exit 1
	(cd ut; ${OUTPUT_DIR_DBG}/ut_vtime) || exit 1
ifndef WITHOUT_STLPORT
	(cd ut; ${OUTPUT_DIR_STLDBG}/ut_vt) || exit 1
endif

check-release-shared:	release-shared
	$(MAKE) -C ut release-shared
	(cd ut; ${OUTPUT_DIR}/ut_vtime) || exit 1

check-dbg-shared:	dbg-shared
	$(MAKE) -C ut dbg-shared
	(cd ut; ${OUTPUT_DIR_DBG}/ut_vtime) || exit 1

ifndef WITHOUT_STLPORT
check-stldbg-shared:	stldbg-shared
	$(MAKE) -C ut stldbg-shared
	(cd ut; ${OUTPUT_DIR_STLDBG}/ut_vtime) || exit 1
endif

depend::
	$(MAKE) -C ut depend

# dbg-shared:	DEFS += -DDEBUG


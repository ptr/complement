# -*- Makefile -*- Time-stamp: <09/10/05 17:14:48 ptr>

SRCROOT := ../..
COMPILER_NAME := gcc

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

INCLUDES += -I$(SRCROOT)/include

LIBMT_DIR = ../mt
LIBSOCK_DIR = ../sockios
LIBSTEM_DIR = ../stem

release-shared: LDFLAGS += -L${LIBMT_DIR}/${OUTPUT_DIR} -L${LIBSOCK_DIR}/${OUTPUT_DIR} -L${LIBSTEM_DIR}/${OUTPUT_DIR}

release-shared:	LDLIBS = -lxmt -lsockios -lstem 

dbg-shared: LDFLAGS += -L${LIBMT_DIR}/${OUTPUT_DIR_DBG} -L${LIBSOCK_DIR}/${OUTPUT_DIR_DBG} -L${LIBSTEM_DIR}/${OUTPUT_DIR_DBG}

dbg-shared:	LDLIBS = -lxmtg -lsockiosg -lstemg

dbg-shared:	DEFS += -D__FIT_VS_TRACE

ifndef WITHOUT_STLPORT
dbg-shared: LDFLAGS += -L${LIBMT_DIR}/${OUTPUT_DIR_STLDBG} -L${LIBSOCK_DIR}/${OUTPUT_DIR_STLDBG} -L${LIBSTEM_DIR}/${OUTPUT_DIR_STLDBG}

stldbg-shared:	LDLIBS = -lxmtstlg -lsockiosstlg -lstemstlg

stldbg-shared:	DEFS += -D__FIT_VS_TRACE
endif

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


# -*- Makefile -*- Time-stamp: <2011-10-07 07:54:13 ptr>

SRCROOT := ../../../..

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

INCLUDES += -I$(SRCROOT)/include -I${BOOST_INCLUDE_DIR}
HEADERS_BASE = $(SRCROOT)/include/net $(SRCROOT)/include/config $(SRCROOT)/include/misc

LIBMT_DIR = ${CoMT_DIR}/lib/mt
LIBSOCK_DIR = ${CoMT_DIR}/lib/sockios
LIBMISC_DIR = ${CoMT_DIR}/lib/misc
LIBSTEM_DIR = ../..
LIBBOOSTRE_DIR = ${CoMT_DIR}/../extern/custom/boost/libs/regex

ifeq ($(OSNAME),linux)

release-shared:	LDFLAGS += -L${LIBMT_DIR}/${OUTPUT_DIR} -L${LIBMISC_DIR}/${OUTPUT_DIR} -L${LIBSOCK_DIR}/${OUTPUT_DIR} -L${LIBBOOSTRE_DIR}/${OUTPUT_DIR}

ifndef WITHOUT_STLPORT

stldbg-shared:	LDFLAGS += -L${LIBMT_DIR}/${OUTPUT_DIR_STLDBG} -L${LIBMISC_DIR}/${OUTPUT_DIR_STLDBG} -L${LIBSOCK_DIR}/${OUTPUT_DIR_STLDBG} -L${LIBBOOSTRE_DIR}/${OUTPUT_DIR_STLDBG}

endif

dbg-shared:	LDFLAGS += -L${LIBMT_DIR}/${OUTPUT_DIR_DBG} -L${LIBMISC_DIR}/${OUTPUT_DIR_DBG} -L${LIBSOCK_DIR}/${OUTPUT_DIR_DBG} -L${LIBBOOSTRE_DIR}/${OUTPUT_DIR_DBG}

endif

release-shared:	LDLIBS += -lxmt -lsockios -lboost_regex -lmisc
dbg-shared:	LDLIBS += -lxmtg -lsockiosg -lboost_regexg -lmiscg
ifndef WITHOUT_STLPORT
stldbg-shared:	LDLIBS += -lxmtstlg -lsockiosstlg -lboost_regexstlg -lmiscstlg
endif


# LDLIBS += -ldl -lbfd
# LDLIBS += -ldl

check:	all-shared
	$(MAKE) -C ut $@

check-release-shared:	release-shared
	$(MAKE) -C ut $@

check-dbg-shared:	dbg-shared
	$(MAKE) -C ut $@

ifndef WITHOUT_STLPORT
check-stldbg-shared:	stldbg-shared
	$(MAKE) -C ut $@
endif

depend clean distclean mostlyclean maintainer-clean::
	${MAKE} -C ut $@

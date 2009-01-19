# -*- Makefile -*- Time-stamp: <07/08/03 23:56:04 ptr>

SRCROOT := ../..

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

INCLUDES += -I$(SRCROOT)/include -I${BOOST_INCLUDE_DIR}
HEADERS_BASE = $(SRCROOT)/include/net $(SRCROOT)/include/config $(SRCROOT)/include/misc

LIBMT_DIR = ${CoMT_DIR}/lib/mt
LIBSOCK_DIR = ${CoMT_DIR}/lib/sockios
LIBSTEM_DIR = ${CoMT_DIR}/lib/stem
LIBMISC_DIR = ${CoMT_DIR}/lib/misc
LIBBOOSTRE_DIR = ${CoMT_DIR}/../extern/custom/boost/libs/regex

ifeq ($(OSNAME),linux)

release-shared:	LDFLAGS += -L${LIBMT_DIR}/${OUTPUT_DIR} -L${LIBMISC_DIR}/${OUTPUT_DIR} -L${LIBSOCK_DIR}/${OUTPUT_DIR} -L${LIBSTEM_DIR}/${OUTPUT_DIR} -L${LIBBOOSTRE_DIR}/${OUTPUT_DIR}

ifndef WITHOUT_STLPORT

stldbg-shared:	LDFLAGS += -L${LIBMT_DIR}/${OUTPUT_DIR_STLDBG} -L${LIBMISC_DIR}/${OUTPUT_DIR_STLDBG} -L${LIBSOCK_DIR}/${OUTPUT_DIR_STLDBG} -L${LIBSTEM_DIR}/${OUTPUT_DIR_STLDBG} -L${LIBBOOSTRE_DIR}/${OUTPUT_DIR_STLDBG}

endif

dbg-shared:	LDFLAGS += -L${LIBMT_DIR}/${OUTPUT_DIR_DBG} -L${LIBMISC_DIR}/${OUTPUT_DIR_DBG} -L${LIBSOCK_DIR}/${OUTPUT_DIR_DBG} -L${LIBSTEM_DIR}/${OUTPUT_DIR_DBG} -L${LIBBOOSTRE_DIR}/${OUTPUT_DIR_DBG}

endif

release-shared:	LDLIBS += -lxmt -lsockios -lstem -lboost_regex -lmisc
dbg-shared:	LDLIBS += -lxmtg -lsockiosg -lstemg -lboost_regexg -lmiscg
ifndef WITHOUT_STLPORT
stldbg-shared:	LDLIBS += -lxmtstlg -lsockiosstlg -lstemstlg -lboost_regexstlg -lmiscstlg
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

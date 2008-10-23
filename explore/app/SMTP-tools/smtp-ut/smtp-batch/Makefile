# -*- Makefile -*- Time-stamp: <04/05/06 14:30:08 ptr>

SRCROOT := ../../../..

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

LIBMT_DIR = ${CoMT_DIR}/lib/mt
LIBSOCKIOS_DIR = ${CoMT_DIR}/lib/sockios
LIBMISC_DIR = ${CoMT_DIR}/lib/misc
LIBBOOSTRE_DIR = ${CoMT_DIR}/../extern/custom/boost/lib/regex

DEFS += -D_NO_TEST
INCLUDES += -I$(SRCROOT)/include -I$(BOOST_INCLUDE_DIR)

release-shared:	LDSEARCH += -L${LIBMT_DIR}/${OUTPUT_DIR} -L${LIBSOCKIOS_DIR}/${OUTPUT_DIR} -L${LIBMISC_DIR}/${OUTPUT_DIR} -L$(LIBBOOSTRE_DIR)/${OUTPUT_DIR}
dbg-shared:	LDSEARCH += -L${LIBMT_DIR}/${OUTPUT_DIR_DBG} -L${LIBSOCKIOS_DIR}/${OUTPUT_DIR_DBG} -L${LIBMISC_DIR}/${OUTPUT_DIR_DBG} -L$(LIBBOOSTRE_DIR)/${OUTPUT_DIR_DBG}
ifndef WITHOUT_STLPORT
stldbg-shared:	LDSEARCH += -L${LIBMT_DIR}/${OUTPUT_DIR_STLDBG} -L${LIBSOCKIOS_DIR}/${OUTPUT_DIR_STLDBG} -L${LIBMISC_DIR}/${OUTPUT_DIR_STLDBG} -L$(LIBBOOSTRE_DIR)/${OUTPUT_DIR_STLDBG}
endif

release-shared : LDLIBS += -lxmt -lsockios -lmisc -lboost_regex
dbg-shared     : LDLIBS += -lxmtg -lsockiosg -lmiscg -lboost_regexg
ifndef WITHOUT_STLPORT
stldbg-shared:	LDLIBS += -lxmtstlg -lsockiosstl -lmiscstlg -lboost_regexstlg
endif

# -*- Makefile -*- Time-stamp: <08/04/23 09:38:04 ptr>

SRCROOT := ../../..

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

LIBMT_DIR = ${CoMT_DIR}/lib/mt
LIBSOCKIOS_DIR = ${CoMT_DIR}/lib/sockios
LIBMISC_DIR = ${CoMT_DIR}/lib/misc
# LIBUTF_DIR = ${CoMT_DIR}/../extern/custom/boost/libs/test/unit_test_framework
# LIBEXAM_DIR = ${CoMT_DIR}/lib/exam
# LIBFS_DIR = ${CoMT_DIR}/../extern/custom/boost/libs/filesystem

DEFS += -D_NO_TEST
INCLUDES += -I$(SRCROOT)/include -I$(STLPORT_INCLUDE_DIR) -I$(BOOST_INCLUDE_DIR)

release-shared:	LDSEARCH += -L${LIBMT_DIR}/${OUTPUT_DIR} -L${LIBSOCKIOS_DIR}/${OUTPUT_DIR} -L${LIBMISC_DIR}/${OUTPUT_DIR}
dbg-shared:	LDSEARCH += -L${LIBMT_DIR}/${OUTPUT_DIR_DBG} -L${LIBSOCKIOS_DIR}/${OUTPUT_DIR_DBG} -L${LIBMISC_DIR}/${OUTPUT_DIR_DBG}
ifndef WITHOUT_STLPORT
stldbg-shared:	LDSEARCH += -L${LIBMT_DIR}/${OUTPUT_DIR_STLDBG} -L${LIBSOCKIOS_DIR}/${OUTPUT_DIR_STLDBG} -L${LIBMISC_DIR}/${OUTPUT_DIR_STLDBG}
endif

release-shared : LDLIBS = -lxmt -lsockios -lmisc
dbg-shared     : LDLIBS = -lxmtg -lsockiosg -lmiscg
ifndef WITHOUT_STLPORT
stldbg-shared  : LDLIBS = -lxmtstlg -lsockiosstlg -lmiscstlg
endif



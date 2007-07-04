# -*- Makefile -*- Time-stamp: <06/08/04 10:54:19 ptr>

SRCROOT := ../..
COMPILER_NAME := gcc

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak


INCLUDES += -I$(SRCROOT)/include -I$(BOOST_INCLUDE_DIR)

# temporary, before dums fix strings:
# DEFS += -D_STLP_DONT_USE_TEMPLATE_EXPRESSION

# RPATH := $(realpath ${CoMT_LIB_DIR})
# ifndef WITHOUT_STLPORT
# RPATH := ${RPATH}:$(realpath ${STLPORT_LIB_DIR}):$(realpath /usr/lib)
# endif

LIBMT_DIR = ${CoMT_DIR}/lib/mt
LIBUTF_DIR = ${CoMT_DIR}/../extern/custom/boost/libs/test/unit_test_framework
LIBFS_DIR = ${CoMT_DIR}/../extern/custom/boost/libs/filesystem

ifeq ($(OSNAME),linux)
release-shared:	LDSEARCH += -L${LIBMT_DIR}/${OUTPUT_DIR} -L${LIBUTF_DIR}/${OUTPUT_DIR} -L${LIBFS_DIR}/${OUTPUT_DIR} -Wl,--rpath=${LIBMT_DIR}/${OUTPUT_DIR}:${LIBUTF_DIR}/${OUTPUT_DIR}:${LIBFS_DIR}/${OUTPUT_DIR}:${STLPORT_LIB_DIR}
stldbg-shared:	LDSEARCH += -L${LIBMT_DIR}/${OUTPUT_DIR_STLDBG} -L${LIBUTF_DIR}/${OUTPUT_DIR_STLDBG} -L${LIBFS_DIR}/${OUTPUT_DIR_STLDBG} -Wl,--rpath=${LIBMT_DIR}/${OUTPUT_DIR_STLDBG}:${LIBUTF_DIR}/${OUTPUT_DIR_STLDBG}:${LIBFS_DIR}/${OUTPUT_DIR_STLDBG}:${STLPORT_LIB_DIR}
dbg-shared:	LDSEARCH += -L${LIBMT_DIR}/${OUTPUT_DIR_DBG} -L${LIBUTF_DIR}/${OUTPUT_DIR_DBG} -L${LIBFS_DIR}/${OUTPUT_DIR_DBG} -Wl,--rpath=${LIBMT_DIR}/${OUTPUT_DIR_DBG}:${LIBUTF_DIR}/${OUTPUT_DIR_DBG}:${LIBFS_DIR}/${OUTPUT_DIR_DBG}:${STLPORT_LIB_DIR}
endif

ifeq ($(OSNAME),openbsd)
release-shared:	LDSEARCH += -L${LIBMT_DIR}/${OUTPUT_DIR} -Wl,-R${LIBMT_DIR}/${OUTPUT_DIR}:${STLPORT_LIB_DIR}
stldbg-shared:	LDSEARCH += -L${LIBMT_DIR}/${OUTPUT_DIR_STLDBG} -Wl,-R${LIBMT_DIR}/${OUTPUT_DIR_STLDBG}:${STLPORT_LIB_DIR}
dbg-shared:	LDSEARCH += -L${LIBMT_DIR}/${OUTPUT_DIR_DBG} -Wl,-R${LIBMT_DIR}/${OUTPUT_DIR_DBG}:${STLPORT_LIB_DIR}
endif

release-shared : LDLIBS = -lxmt -lboost_test_utf -lboost_fs
stldbg-shared  : LDLIBS = -lxmtstlg -lboost_test_utfstlg -lboost_fsstlg
dbg-shared     : LDLIBS = -lxmtg -lboost_test_utfg -lboost_fsg

ifeq ($(OSNAME),freebsd)
release-shared : LDLIBS += -lthr
stldbg-shared  : LDLIBS += -lthr
dbg-shared     : LDLIBS += -lthr
endif

ifeq ($(OSNAME),sunos)
release-shared : LDLIBS += -lrt
stldbg-shared  : LDLIBS += -lrt
dbg-shared     : LDLIBS += -lrt
endif



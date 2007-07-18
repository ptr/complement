# -*- Makefile -*- Time-stamp: <07/07/17 10:01:41 ptr>

SRCROOT := ../..
COMPILER_NAME := gcc

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak


INCLUDES += -I$(SRCROOT)/include -I$(BOOST_INCLUDE_DIR)
DEFS += -D__FIT_EXAM

# temporary, before dums fix strings:
# DEFS += -D_STLP_DONT_USE_TEMPLATE_EXPRESSION

# RPATH := $(realpath ${CoMT_LIB_DIR})
# ifndef WITHOUT_STLPORT
# RPATH := ${RPATH}:$(realpath ${STLPORT_LIB_DIR}):$(realpath /usr/lib)
# endif

LIBMT_DIR = ${CoMT_DIR}/lib/mt
# LIBUTF_DIR = ${CoMT_DIR}/../extern/custom/boost/libs/test/unit_test_framework
LIBEXAM_DIR = ${CoMT_DIR}/lib/exam
LIBFS_DIR = ${CoMT_DIR}/../extern/custom/boost/libs/filesystem

ifeq ($(OSNAME),linux)
release-shared:	LDSEARCH += -L${LIBMT_DIR}/${OUTPUT_DIR} -L${LIBEXAM_DIR}/${OUTPUT_DIR} -L${LIBFS_DIR}/${OUTPUT_DIR} -Wl,--rpath=${LIBMT_DIR}/${OUTPUT_DIR}:${LIBEXAM_DIR}/${OUTPUT_DIR}:${LIBFS_DIR}/${OUTPUT_DIR}:${STLPORT_LIB_DIR}
stldbg-shared:	LDSEARCH += -L${LIBMT_DIR}/${OUTPUT_DIR_STLDBG} -L${LIBEXAM_DIR}/${OUTPUT_DIR_STLDBG} -L${LIBFS_DIR}/${OUTPUT_DIR_STLDBG} -Wl,--rpath=${LIBMT_DIR}/${OUTPUT_DIR_STLDBG}:${LIBEXAM_DIR}/${OUTPUT_DIR_STLDBG}:${LIBFS_DIR}/${OUTPUT_DIR_STLDBG}:${STLPORT_LIB_DIR}
dbg-shared:	LDSEARCH += -L${LIBMT_DIR}/${OUTPUT_DIR_DBG} -L${LIBEXAM_DIR}/${OUTPUT_DIR_DBG} -L${LIBFS_DIR}/${OUTPUT_DIR_DBG} -Wl,--rpath=${LIBMT_DIR}/${OUTPUT_DIR_DBG}:${LIBEXAM_DIR}/${OUTPUT_DIR_DBG}:${LIBFS_DIR}/${OUTPUT_DIR_DBG}:${STLPORT_LIB_DIR}
endif

ifeq ($(OSNAME),openbsd)
release-shared:	LDSEARCH += -L${LIBMT_DIR}/${OUTPUT_DIR} -Wl,-R${LIBMT_DIR}/${OUTPUT_DIR}:${STLPORT_LIB_DIR}
stldbg-shared:	LDSEARCH += -L${LIBMT_DIR}/${OUTPUT_DIR_STLDBG} -Wl,-R${LIBMT_DIR}/${OUTPUT_DIR_STLDBG}:${STLPORT_LIB_DIR}
dbg-shared:	LDSEARCH += -L${LIBMT_DIR}/${OUTPUT_DIR_DBG} -Wl,-R${LIBMT_DIR}/${OUTPUT_DIR_DBG}:${STLPORT_LIB_DIR}
endif

release-shared : LDLIBS = -lxmt -lexam -lboost_fs
stldbg-shared  : LDLIBS = -lxmtstlg -lexamstlg -lboost_fsstlg
dbg-shared     : LDLIBS = -lxmtg -lexamg -lboost_fsg

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



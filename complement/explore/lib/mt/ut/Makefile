# -*- Makefile -*- Time-stamp: <08/06/12 15:11:40 ptr>

SRCROOT := ../../..

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak


INCLUDES += -I$(SRCROOT)/include
ifdef BOOST_DIR
INCLUDES += -I$(BOOST_INCLUDE_DIR)
endif
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

release-shared:	LDFLAGS += -L${LIBMT_DIR}/${OUTPUT_DIR} -L${LIBEXAM_DIR}/${OUTPUT_DIR} -L${LIBFS_DIR}/${OUTPUT_DIR} -Wl,--rpath=${LIBMT_DIR}/${OUTPUT_DIR}:${LIBEXAM_DIR}/${OUTPUT_DIR}:${LIBFS_DIR}/${OUTPUT_DIR}:${STLPORT_LIB_DIR}

dbg-shared:	LDFLAGS += -L${LIBMT_DIR}/${OUTPUT_DIR_DBG} -L${LIBEXAM_DIR}/${OUTPUT_DIR_DBG} -L${LIBFS_DIR}/${OUTPUT_DIR_DBG} -Wl,--rpath=${LIBMT_DIR}/${OUTPUT_DIR_DBG}:${LIBEXAM_DIR}/${OUTPUT_DIR_DBG}:${LIBFS_DIR}/${OUTPUT_DIR_DBG}:${STLPORT_LIB_DIR}

ifndef WITHOUT_STLPORT
stldbg-shared:	LDFLAGS += -L${LIBMT_DIR}/${OUTPUT_DIR_STLDBG} -L${LIBEXAM_DIR}/${OUTPUT_DIR_STLDBG} -L${LIBFS_DIR}/${OUTPUT_DIR_STLDBG} -Wl,--rpath=${LIBMT_DIR}/${OUTPUT_DIR_STLDBG}:${LIBEXAM_DIR}/${OUTPUT_DIR_STLDBG}:${LIBFS_DIR}/${OUTPUT_DIR_STLDBG}:${STLPORT_LIB_DIR}
endif

endif

ifeq ($(OSNAME),openbsd)

release-shared:	LDFLAGS += -L${LIBMT_DIR}/${OUTPUT_DIR} -Wl,-R${LIBMT_DIR}/${OUTPUT_DIR}:${STLPORT_LIB_DIR}

dbg-shared:	LDFLAGS += -L${LIBMT_DIR}/${OUTPUT_DIR_DBG} -Wl,-R${LIBMT_DIR}/${OUTPUT_DIR_DBG}:${STLPORT_LIB_DIR}

ifndef WITHOUT_STLPORT
stldbg-shared:	LDFLAGS += -L${LIBMT_DIR}/${OUTPUT_DIR_STLDBG} -Wl,-R${LIBMT_DIR}/${OUTPUT_DIR_STLDBG}:${STLPORT_LIB_DIR}
endif

endif

release-shared : LDLIBS = -lxmt -lexam -lboost_filesystem
dbg-shared     : LDLIBS = -lxmtg -lexamg -lboost_fsg
ifndef WITHOUT_STLPORT
stldbg-shared  : LDLIBS = -lxmtstlg -lexamstlg -lboost_fsstlg
endif

ifeq ($(OSNAME),freebsd)
release-shared : LDLIBS += -lthr
dbg-shared     : LDLIBS += -lthr
ifndef WITHOUT_STLPORT
stldbg-shared  : LDLIBS += -lthr
endif
endif

ifeq ($(OSNAME),sunos)
release-shared : LDLIBS += -lrt
dbg-shared     : LDLIBS += -lrt
ifndef WITHOUT_STLPORT
stldbg-shared  : LDLIBS += -lrt
endif
endif



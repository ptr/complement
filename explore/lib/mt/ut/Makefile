# -*- Makefile -*-

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
LIBEXAM_DIR = ${CoMT_DIR}/lib/exam
LIBMISC_DIR = ${CoMT_DIR}/lib/misc
# LIBFS_DIR = ${CoMT_DIR}/../extern/custom/boost/libs/filesystem
CXXFLAGS += -Wall

ifeq ($(OSNAME),linux)

release-shared:	LDFLAGS += -L${LIBMT_DIR}/${OUTPUT_DIR} -L${LIBEXAM_DIR}/${OUTPUT_DIR} -L${LIBFS_DIR}/${OUTPUT_DIR} -L${LIBMISC_DIR}/${OUTPUT_DIR} -Wl,--rpath=${LIBMT_DIR}/${OUTPUT_DIR}:${LIBEXAM_DIR}/${OUTPUT_DIR}:${LIBMISC_DIR}/${OUTPUT_DIR}:${STLPORT_LIB_DIR}

dbg-shared:	LDFLAGS += -L${LIBMT_DIR}/${OUTPUT_DIR_DBG} -L${LIBEXAM_DIR}/${OUTPUT_DIR_DBG} -L${LIBFS_DIR}/${OUTPUT_DIR_DBG} -L${LIBMISC_DIR}/${OUTPUT_DIR_DBG} -Wl,--rpath=${LIBMT_DIR}/${OUTPUT_DIR_DBG}:${LIBEXAM_DIR}/${OUTPUT_DIR_DBG}:${LIBMISC_DIR}/${OUTPUT_DIR_DBG}:${STLPORT_LIB_DIR}

ifndef WITHOUT_STLPORT
stldbg-shared:	LDFLAGS += -L${LIBMT_DIR}/${OUTPUT_DIR_STLDBG} -L${LIBEXAM_DIR}/${OUTPUT_DIR_STLDBG} -L${LIBFS_DIR}/${OUTPUT_DIR_STLDBG} -L${LIBMISC_DIR}/${OUTPUT_DIR_STLDBG} -Wl,--rpath=${LIBMT_DIR}/${OUTPUT_DIR_STLDBG}:${LIBEXAM_DIR}/${OUTPUT_DIR_STLDBG}:${LIBMISC_DIR}/${OUTPUT_DIR_STLDBG}:${STLPORT_LIB_DIR}
endif

endif

ifeq ($(OSNAME),openbsd)

release-shared:	LDFLAGS += -L${LIBMT_DIR}/${OUTPUT_DIR} -Wl,-R${LIBMT_DIR}/${OUTPUT_DIR}:${STLPORT_LIB_DIR}

dbg-shared:	LDFLAGS += -L${LIBMT_DIR}/${OUTPUT_DIR_DBG} -Wl,-R${LIBMT_DIR}/${OUTPUT_DIR_DBG}:${STLPORT_LIB_DIR}

ifndef WITHOUT_STLPORT
stldbg-shared:	LDFLAGS += -L${LIBMT_DIR}/${OUTPUT_DIR_STLDBG} -Wl,-R${LIBMT_DIR}/${OUTPUT_DIR_STLDBG}:${STLPORT_LIB_DIR}
endif

endif

release-shared : LDLIBS = -lxmt -lexam -lmisc
dbg-shared     : LDLIBS_DBG = -lxmtg -lexamg -lmiscg
ifndef WITHOUT_STLPORT
stldbg-shared  : LDLIBS_STLDBG = -lxmtstlg -lexamstlg -lmiscstlg
endif

ifeq ($(OSNAME),freebsd)
release-shared : STDLIBS += -lthr
dbg-shared     : STDLIBS += -lthr
ifndef WITHOUT_STLPORT
stldbg-shared  : STDLIBS += -lthr
endif
endif

ifeq ($(OSNAME),sunos)
release-shared : STDLIBS += -lrt
dbg-shared     : STDLIBS += -lrt
ifndef WITHOUT_STLPORT
stldbg-shared  : STDLIBS += -lrt
endif
endif

check-release-shared:   release-shared
	${PRG}

check-dbg-shared:       dbg-shared
	${PRG_DBG}

ifndef WITHOUT_STLPORT
check-stldbg-shared:    stldbg-shared
	${PRG_STLDBG}
endif

check:	all-shared
	@${PRG}
	@${PRG_DBG}
ifndef WITHOUT_STLPORT
	@${PRG_STLDBG}
endif

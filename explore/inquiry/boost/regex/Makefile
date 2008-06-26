# -*- Makefile -*- Time-stamp: <03/11/21 08:02:55 ptr>

SRCROOT := ../../..
COMPILER_NAME := gcc

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

# INCLUDES += -I$(SRCROOT)/include -I$(STLPORT_INCLUDE_DIR) -I$(BOOST_INCLUDE_DIR)

# temporary, before dums fix strings:
# DEFS += -D_STLP_DONT_USE_TEMPLATE_EXPRESSION

LIBBOOST_RE_DIR = ${CoMT_DIR}/../extern/custom/boost/libs/regex
# LIBUTF_DIR = ${CoMT_DIR}/../extern/custom/boost/libs/test/unit_test_framework

ifeq ($(OSNAME),linux)
release-shared: LDSEARCH += -L${LIBBOOST_RE_DIR}/${OUTPUT_DIR} -Wl,--rpath=${LIBBOOST_RE_DIR}/${OUTPUT_DIR}:${STLPORT_LIB_DIR}
ifndef WITHOUT_STLPORT
stldbg-shared:  LDSEARCH += -L${LIBBOOST_RE_DIR}/${OUTPUT_DIR_STLDBG} -Wl,--rpath=${LIBBOOST_RE_DIR}/${OUTPUT_DIR_STLDBG}:${STLPORT_LIB_DIR}
endif
dbg-shared:     LDSEARCH += -L${LIBBOOST_RE_DIR}/${OUTPUT_DIR_DBG} -Wl,--rpath=${LIBBOOST_RE_DIR}/${OUTPUT_DIR_DBG}:${STLPORT_LIB_DIR}
endif

release-shared : LDLIBS = -lboost_regex
ifndef WITHOUT_STLPORT
stldbg-shared  : LDLIBS = -lboost_regexstlg
endif
dbg-shared     : LDLIBS = -lboost_regexg

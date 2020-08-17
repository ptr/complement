# -*- Makefile -*-

SRCROOT := ../../../..
COMPILER_NAME := gcc
# ALL_TAGS := install-release-shared install-dbg-shared
# CoMT_DIR := ../../external/complement/explore

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

LIBMT_DIR = ${CoMT_DIR}/lib/mt
LIBMISC_DIR = ${CoMT_DIR}/lib/misc
LIBSOCK_DIR = ${CoMT_DIR}/lib/sockios
LIBSTEM_DIR = ${CoMT_DIR}/lib/stem

# DEFS += -DUNIT_TEST
INCLUDES += -I${CoMT_INCLUDE_DIR}

LDLIBS = -lxmt -lmisc -lsockios -lstem
LDLIBS_DBG = lxmtg -lmisc -lsockiosg -lstemg

release-shared: LDFLAGS += -L${LIBMT_DIR}/${OUTPUT_DIR} -L${LIBMISC_DIR}/${OUTPUT_DIR} -L${LIBSOCK_DIR}/${OUTPUT_DIR} -L${LIBSTEM_DIR}/${OUTPUT_DIR} -Wl,--rpath=${LIBMT_DIR}/${OUTPUT_DIR}:${LIBMISC_DIR}/${OUTPUT_DIR}:${LIBSOCK_DIR}/${OUTPUT_DIR}:${LIBSTEM_DIR}/${OUTPUT_DIR}

dbg-shared:     LDFLAGS += -L${LIBMT_DIR}/${OUTPUT_DIR_DBG} -L${LIBMISC_DIR}/${OUTPUT_DIR_DBG} -L${LIBSOCK_DIR}/${OUTPUT_DIR_DBG} -L${LIBSTEM_DIR}/${OUTPUT_DIR_DBG} -Wl,--rpath=${LIBMT_DIR}/${OUTPUT_DIR_DBG}:${LIBMISC_DIR}/${OUTPUT_DIR_DBG}:${LIBSOCK_DIR}/${OUTPUT_DIR_DBG}:${LIBSTEM_DIR}/${OUTPUT_DIR_DBG}

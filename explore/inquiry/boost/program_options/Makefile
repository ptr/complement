# -*- Makefile -*- Time-stamp: <03/11/21 08:02:55 ptr>

SRCROOT := ../../..
COMPILER_NAME := gcc

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak


INCLUDES += -I$(SRCROOT)/include -I$(BOOST_INCLUDE_DIR)

# temporary, before dums fix strings:
# DEFS += -D_STLP_DONT_USE_TEMPLATE_EXPRESSION

LDFLAGS += -L${CoMT_LIB_DIR} -Wl,--rpath=${CoMT_LIB_DIR}:${STLPORT_LIB_DIR}

release-shared : LDLIBS = -lboost_program_options
stldbg-shared  : LDLIBS = -lboost_program_optionsstlg
dbg-shared     : LDLIBS = -lboost_program_optionsg

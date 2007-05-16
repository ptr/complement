# -*- Makefile -*- Time-stamp: <07/02/07 12:28:46 ptr>

SRCROOT := ../..
COMPILER_NAME := gcc

include Makefile.inc
include ${SRCROOT}/Makefiles/top.mak

INCLUDES += -I$(SRCROOT)/include -I$(BOOST_INCLUDE_DIR)

LIBMT_DIR = ${CoMT_DIR}/lib/mt
LIBSOCK_DIR = ${CoMT_DIR}/lib/sockios
LIBSTEM_DIR = ${CoMT_DIR}/lib/stem

LDSEARCH += -L${CoMT_LIB_DIR} -Wl,--rpath=${CoMT_LIB_DIR}

release-shared : LDLIBS = -lxmt -lsockios -lstem 
stldbg-shared  : LDLIBS = -lxmtstlg -lsockiosstlg -lstemstlg
dbg-shared     : LDLIBS = -lxmtg -lsockiosg -lstemg

# dbg-shared:	DEFS += -DDEBUG


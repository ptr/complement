# -*- Makefile -*- Time-stamp: <04/05/06 14:30:08 ptr>
# $Id: Makefile,v 1.1 2005/05/10 15:13:23 ptr Exp $

SRCROOT := ../../../../../../island/workshop/explore
COMPILER_NAME := gcc

include Makefile.inc
include ${SRCROOT}/Makefiles/top.mak


DEFS += -D_NO_TEST
INCLUDES += -I$(SRCROOT)/include -I$(STLPORT_INCLUDE_DIR) -I$(BOOST_INCLUDE_DIR)

release-shared:	LDSEARCH = -L${STLPORT_LIB_DIR} -L${CoMT_LIB_DIR} -Wl,-rpath=${STLPORT_LIB_DIR}
stldbg-shared:	LDSEARCH = -L${STLPORT_LIB_DIR} -L${CoMT_LIB_DIR_STLDBG}
dbg-shared:	LDSEARCH = -L${STLPORT_LIB_DIR} -L${CoMT_LIB_DIR_DBG}

release-shared : LDLIBS = -lxmt -lsockios -lmisc
stldbg-shared  : LDLIBS = -lxmtstlg -lsockiosstlg -lmiscstlg
dbg-shared     : LDLIBS = -lxmtg -lsockiosg -lmiscg

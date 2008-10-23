# -*- Makefile -*- Time-stamp: <08/04/23 10:21:52 ptr>

SRCROOT := ../../../..

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

INCLUDES += -I$(SRCROOT)/include -I$(BOOST_INCLUDE_DIR) -I..

release-shared:	LDSEARCH += -L${CoMT_LIB_DIR}
dbg-shared:	LDSEARCH += -L${CoMT_LIB_DIR_DBG}
ifndef WITHOUT_STLPORT
stldbg-shared:	LDSEARCH += -L${CoMT_LIB_DIR_STLDBG}
endif

release-shared : LDLIBS = -lboost_test_utf
dbg-shared     : LDLIBS = -lboost_test_utfg
ifndef WITHOUT_STLPORT
stldbg-shared  : LDLIBS = -lboost_test_utfstlg
endif

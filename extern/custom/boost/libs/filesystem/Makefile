# -*- makefile -*- Time-stamp: <06/06/27 02:25:13 ptr>

SRCROOT := ../../../../../explore

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

ifdef USE_SYSTEM_BOOST
POST_INSTALL += ; rm -f $(INSTALL_LIB_DIR)/${SO_NAME}
POST_INSTALL_DBG += ; rm -f $(INSTALL_LIB_DIR_DBG)/${SO_NAME_DBG}
endif

INCLUDES += -I${BOOST_INCLUDE_DIR}

check:	all
	${MAKE} -C test-v2 check

check-release-shared:	release-shared
	${MAKE} -C test-v2 check-release-shared

check-dbg-shared:	dbg-shared
	${MAKE} -C test-v2 check-dbg-shared

ifndef WITHOUT_STLPORT
check-stldbg-shared:	stldbg-shared
	${MAKE} -C test-v2 check-stldbg-shared
endif


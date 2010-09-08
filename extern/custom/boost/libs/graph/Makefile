# -*- makefile -*- Time-stamp: <06/06/27 02:25:13 ptr>

SRCROOT := ../../../../../explore
#POST_INSTALL = ln -sf ${SO_NAME} $(INSTALL_LIB_DIR)/libboost_filesystem.so
#POST_INSTALL_DBG = ln -sf ${SO_NAME_DBG} $(INSTALL_LIB_DIR_DBG)/libboost_filesystem.so

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

ifdef USE_SYSTEM_BOOST
#POST_INSTALL += ; rm -f  $(INSTALL_LIB_DIR)/libboost_filesystem.so $(INSTALL_LIB_DIR)/${SO_NAME}
#POST_INSTALL_DBG += ; rm -f  $(INSTALL_LIB_DIR_DBG)/libboost_filesystem.so $(INSTALL_LIB_DIR_DBG)/${SO_NAME_DBG}
endif

INCLUDES += -I${BOOST_INCLUDE_DIR}

check:	all
	${MAKE} -C test check

check-release-shared:	release-shared
	${MAKE} -C test check-release-shared

check-dbg-shared:	dbg-shared
	${MAKE} -C test check-dbg-shared

ifndef WITHOUT_STLPORT
check-stldbg-shared:	stldbg-shared
	${MAKE} -C test check-stldbg-shared
endif


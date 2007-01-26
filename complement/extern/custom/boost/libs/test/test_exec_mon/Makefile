# -*- makefile -*- Time-stamp: <07/01/24 22:12:49 ptr>

SRCROOT := ../../../../../../explore
COMPILER_NAME := gcc
POST_INSTALL = ln -sf ${SO_NAME} $(INSTALL_LIB_DIR)/libboost_unit_test_framework.so
POST_INSTALL_DBG = ln -sf ${SO_NAME_DBG} $(INSTALL_LIB_DIR_DBG)/libboost_unit_test_framework.so

include Makefile.inc
include ${SRCROOT}/Makefiles/top.mak

INCLUDES += -I${BOOST_INCLUDE_DIR}

ifdef USE_SYSTEM_BOOST
POST_INSTALL += ; rm -f  $(INSTALL_LIB_DIR)/libboost_unit_test_framework.so $(INSTALL_LIB_DIR)/${SO_NAME}
POST_INSTALL_DBG += ; rm -f  $(INSTALL_LIB_DIR_DBG)/libboost_unit_test_framework.so $(INSTALL_LIB_DIR_DBG)/${SO_NAME_DBG}
endif

check:	all-shared
	$(MAKE) -C test check || exit 1

check-release-shared:	release-shared
	$(MAKE) -C test check-release-shared || exit 1

check-dbg-shared:	dbg-shared
	$(MAKE) -C test check-dbg-shared || exit 1

ifndef WITHOUT_STLPORT
check-stldbg-shared:	stldbg-shared
	$(MAKE) -C test check-stldbg-shared || exit 1
endif

depend::
	$(MAKE) -C test depend

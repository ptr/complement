# -*- makefile -*- Time-stamp: <06/06/27 02:26:25 ptr>

SRCROOT := ../../../../../../explore
BOOST_DIR := ${HOME}/labs/boost
# WITHOUT_STLPORT := 1
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

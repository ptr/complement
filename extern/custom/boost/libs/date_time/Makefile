# -*- makefile -*- Time-stamp: <10/03/15 23:12:54 ptr>

SRCROOT := ../../../../../explore
COMPILER_NAME := gcc
POST_INSTALL = ln -sf ${SO_NAME} $(INSTALL_LIB_DIR)/libboost_date_time.so
#POST_INSTALL_DBG = ln -sf ${SO_NAME_DBG} $(INSTALL_LIB_DIR_DBG)/libboost_date_time.so

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

INCLUDES += -I${BOOST_INCLUDE_DIR}

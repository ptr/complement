# -*- makefile -*- Time-stamp: <03/03/31 15:18:29 ptr>

SRCROOT := ../../../../../explore
COMPILER_NAME := gcc
POST_INSTALL = ln -sf ${SO_NAME} $(INSTALL_LIB_DIR)/libboost_serialization-gcc-mt.so
# POST_INSTALL_DBG = ln -sf ${SO_NAME_DBG} $(INSTALL_LIB_DIR_DBG)/libboost_serialization.so

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

INCLUDES += -I${BOOST_INCLUDE_DIR}

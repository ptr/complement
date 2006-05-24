# Time-stamp: <05/09/02 00:48:09 ptr>

# This is Complement project (really not extern):

CoMT_LIB_DIR        ?= ${INSTALL_LIB_DIR}
CoMT_LIB_DIR_DBG    ?= ${INSTALL_LIB_DIR_DBG}
CoMT_LIB_DIR_STLDBG ?= ${INSTALL_LIB_DIR_STLDBG}
CoMT_BIN_DIR        ?= ${INSTALL_BIN_DIR}
CoMT_BIN_DIR_DBG    ?= ${INSTALL_BIN_DIR_DBG}
CoMT_BIN_DIR_STLDBG ?= ${INSTALL_BIN_DIR_STLDBG}

CoMT_INCLUDE_DIR ?= ${CoMT_DIR}/include

# This file reflect versions of third-party libraries that
# used in projects

# STLport library
STLPORT_LIB_DIR ?= $(STLPORT_DIR)/${TARGET_NAME}lib
STLPORT_INCLUDE_DIR ?= $(STLPORT_DIR)/stlport

# boost (http://www.boost.org, http://boost.sourceforge.net)
BOOST_INCLUDE_DIR ?= ${BOOST_DIR}

BOOST_DIR   ?= ${SRCROOT}/../extern/boost
STLPORT_DIR ?= /export/home/ptr/STLport.lab/STLport
CoMT_DIR    ?= ${SRCROOT}

# Time-stamp: <05/06/03 21:34:35 ptr>
# $Id$

MWCW_BASE ?= c:/Program Files/Metrowerks/CodeWarrior
NWSDK_DIR ?= c:/Novell/ndk/nwsdk

# STLport library
STLPORT_LIB_DIR ?= $(STLPORT_DIR)/${TARGET_NAME}lib
STLPORT_INCLUDE_DIR ?= $(STLPORT_DIR)/stlport

MWCW_NOVELL = $(MWCW_BASE)/Novell Support/Metrowerks Support
MWCW_NOVELL_SDK = $(NWSDK_DIR)

# This is Complement project (really not extern):

CoMT_LIB_DIR        ?= ${INSTALL_LIB_DIR}
CoMT_LIB_DIR_DBG    ?= ${INSTALL_LIB_DIR_DBG}
CoMT_LIB_DIR_STLDBG ?= ${INSTALL_LIB_DIR_STLDBG}
CoMT_BIN_DIR        ?= ${INSTALL_BIN_DIR}
CoMT_BIN_DIR_DBG    ?= ${INSTALL_BIN_DIR_DBG}
CoMT_BIN_DIR_STLDBG ?= ${INSTALL_BIN_DIR_STLDBG}

CoMT_INCLUDE_DIR ?= ${CoMT_DIR}/include

# boost (http://www.boost.org, http://boost.sourceforge.net)
BOOST_INCLUDE_DIR ?= ${BOOST_DIR}


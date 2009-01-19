# Time-stamp: <08/06/06 17:06:16 yeti>
#
# Copyright (c) 1997-1999, 2002, 2003, 2005, 2006, 2008
# Petr Ovtchenkov
#
# Portion Copyright (c) 1999-2001
# Parallel Graphics Ltd.
#
# Licensed under the Academic Free License version 3.0
#

# Complement project:

CoMT_DIR            ?= ${SRCROOT}

CoMT_LIB_DIR        ?= ${INSTALL_LIB_DIR}
CoMT_LIB_DIR_DBG    ?= ${INSTALL_LIB_DIR_DBG}
CoMT_LIB_DIR_STLDBG ?= ${INSTALL_LIB_DIR_STLDBG}
CoMT_BIN_DIR        ?= ${INSTALL_BIN_DIR}
CoMT_BIN_DIR_DBG    ?= ${INSTALL_BIN_DIR_DBG}
CoMT_BIN_DIR_STLDBG ?= ${INSTALL_BIN_DIR_STLDBG}

CoMT_INCLUDE_DIR    ?= ${CoMT_DIR}/include

# boost (http://www.boost.org, http://boost.sourceforge.net)

ifdef BOOST_DIR
BOOST_INCLUDE_DIR ?= ${BOOST_DIR}
endif

# STLport library

ifndef STLPORT_DIR
ifndef STLPORT_INCLUDE_DIR
ifndef WITHOUT_STLPORT
WITHOUT_STLPORT = 1
endif
endif
endif

ifdef STLPORT_DIR
STLPORT_INCLUDE_DIR ?= $(STLPORT_DIR)/stlport
endif

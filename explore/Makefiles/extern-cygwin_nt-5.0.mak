# Time-stamp: <03/07/10 15:33:20 ptr>
# $Id$

# This file reflect versions of third-party libraries that
# used in projects

# STLport library
#STLPORT_LIB_DIR ?= /usr/local/lib
#STLPORT_INCLUDE_DIR ?= /usr/local/include/stlport
#STLPORT_VER ?= 4.5
STLPORT_LIB_DIR ?= $(STLPORT_DIR)/lib
STLPORT_INCLUDE_DIR ?= $(STLPORT_DIR)/stlport
STLPORT_VER ?= 4.5.5

# PostgreSQL library version:

PG_INCLUDE ?= $(PG_DIR)/include
PG_LIB ?= $(PG_DIR)/lib
PG_LIB_VER_MAJOR = 2
PG_LIB_VER_MINOR = 1

# Readline libraries version:

RL_INCLUDE ?= /usr/local/include/readline
RL_LIB ?= /usr/local/lib
RL_LIB_VER_MAJOR = 4
RL_LIB_VER_MINOR = 2

# gSOAP (http://gsoap2.sourceforge.net)

gSOAP_INCLUDE_DIR ?= ${gSOAP_DIR}/include
gSOAP_LIB_DIR ?= ${gSOAP_DIR}/lib
gSOAP_BIN_DIR ?= ${gSOAP_DIR}/bin

# boost (http://www.boost.org, http://boost.sourceforge.net)
BOOST_INCLUDE_DIR ?= ${BOOST_DIR}

# This file reflect versions of third-party libraries that
# used in projects, with make-depend style

ifeq ($(OSNAME),sunos)
PG_DIR ?= /opt/PGpgsql
endif
ifeq ($(OSNAME),linux)
PG_DIR ?= /usr/local/pgsql
endif

gSOAP_DIR ?= /opt/gSOAP-2.2.3
BOOST_DIR ?= ${SRCROOT}/../extern/boost
STLPORT_DIR ?= e:/STLlab/STLport

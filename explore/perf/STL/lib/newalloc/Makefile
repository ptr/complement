# -*- Makefile -*- Time-stamp: <05/12/27 00:05:24 ptr>

STLPORT_SRC_DIR := /export/home/ptr/STLport.lab/STLport
SRCROOT := ${STLPORT_SRC_DIR}/build
INSTALL_LIB_DIR        := ../../build/lib/newalloc
INSTALL_LIB_DIR_DBG    := .
INSTALL_LIB_DIR_STLDBG := .
INSTALL_BIN_DIR        := .
INSTALL_BIN_DIR_DBG    := .
INSTALL_BIN_DIR_STLDBG := .
ALL_TAGS := install-release-shared

COMPILER_NAME := gcc

STLPORT_INCLUDE_DIR = ${STLPORT_SRC_DIR}/stlport
include ../Makefile.inc
include ${SRCROOT}/Makefiles/top.mak

ifeq ($(OSNAME),linux)
DEFS += -D_STLP_REAL_LOCALE_IMPLEMENTED -D_GNU_SOURCE -D_STLP_LEAKS_PEDANTIC
endif

DEFS += -D_STLP_USE_NEWALLOC

# options for build with boost support
ifdef STLP_BUILD_BOOST_PATH
INCLUDES += -I$(STLP_BUILD_BOOST_PATH)
endif


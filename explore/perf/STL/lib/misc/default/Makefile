# -*- Makefile -*- Time-stamp: <03/07/09 18:08:47 ptr>
# $Id$

SRCROOT := ../../../../..
COMPILER_NAME := gcc
STLPORT_SRC_DIR := /export/home/ptr/STLport.lab/cvs.stlport.com/STLport
STLPORT_INCLUDE_DIR := $(STLPORT_SRC_DIR)/stlport
INSTALL_LIB_DIR     := ../../../build/lib
ALL_TAGS := install-release-shared


include ../Makefile.inc
include ${SRCROOT}/Makefiles/top.mak

INCLUDES += -I$(SRCROOT)/include -I$(STLPORT_INCLUDE_DIR)

ifeq ($(OSNAME),linux)
release-shared:	LDSEARCH = -L${INSTALL_LIB_DIR} -Wl,-rpath=${INSTALL_LIB_DIR}
stldbg-shared:	LDSEARCH = -L${STLPORT_LIB_DIR}
dbg-shared:	LDSEARCH = -L${STLPORT_LIB_DIR}
endif


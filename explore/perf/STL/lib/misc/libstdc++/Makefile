# -*- Makefile -*- Time-stamp: <06/01/03 17:28:09 ptr>

SRCROOT := ../../../../..
COMPILER_NAME := gcc
STLPORT_SRC_DIR := /export/home/ptr/STLport.lab/STLport
STLPORT_INCLUDE_DIR := $(STLPORT_SRC_DIR)/stlport
INSTALL_LIB_DIR     := ../../../build/lib/libstdc++
ALL_TAGS := install-release-shared

WITHOUT_STLPORT := 1

include ../Makefile.inc
include ${SRCROOT}/Makefiles/top.mak

INCLUDES := -I${CoMT_INCLUDE_DIR}

ifeq ($(OSNAME),linux)
release-shared:	LDSEARCH = -L${INSTALL_LIB_DIR} -Wl,-rpath=${INSTALL_LIB_DIR}
endif


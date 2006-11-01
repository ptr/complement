# -*- Makefile -*- Time-stamp: <06/06/28 14:47:11 ptr>

SRCROOT := ../..
COMPILER_NAME := gcc

include Makefile.inc
include ${SRCROOT}/Makefiles/top.mak

INCLUDES += -I$(SRCROOT)/include

ifdef WITHOUT_STLPORT
INCLUDES += -I${BOOST_INCLUDE_DIR}
endif

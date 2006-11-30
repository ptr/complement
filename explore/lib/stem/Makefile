# -*- Makefile -*- Time-stamp: <06/11/30 22:27:35 ptr>

SRCROOT := ../..
COMPILER_NAME := gcc

include Makefile.inc
include ${SRCROOT}/Makefiles/top.mak

INCLUDES += -I$(SRCROOT)/include

dbg-shared:	DEFS += -D__FIT_STEM_TRACE=1

ifdef WITHOUT_STLPORT
INCLUDES += -I${BOOST_INCLUDE_DIR}
else
stldbg-shared:	DEFS += -D__FIT_STEM_TRACE=1
endif

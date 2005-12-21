# -*- makefile -*- Time-stamp: <05/04/01 00:06:12 ptr>
# $Id$

SRCROOT := ../../../
COMPILER_NAME = gcc
include Makefile.inc
include ${SRCROOT}/Makefiles/top.mak

INCLUDES += -I$(STLPORT_INCLUDE_DIR)

LDLIBS += -lrt -lnsl -lfl
LDFLAGS += -L${STLPORT_LIB_DIR}

lex.yy.cc:	rtf.lex
	flex -+ rtf.lex

# -*- Makefile -*- Time-stamp: <05/06/03 21:45:55 ptr>
# $Id$

SRCROOT := ../..
COMPILER_NAME := mwccnlm

STLPORT_DIR := ../../../Novell-STLP/STLport
include Makefile.inc
include ${SRCROOT}/Makefiles/top.mak

INCLUDES += -I$(SRCROOT)/include -I$(STLPORT_INCLUDE_DIR)

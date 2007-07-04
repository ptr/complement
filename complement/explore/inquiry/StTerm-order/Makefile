# -*- Makefile -*- Time-stamp: <03/07/09 18:08:47 ptr>
# $Id$

SRCROOT := ../..
COMPILER_NAME := gcc

STLPORT_DIR := /export/home/ptr/STLport.lab/cvs.stlport.com/STLport
include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

#CXXFLAGS += -fuse-cxa-atexit
LDFLAGS += -Wl,-rpath=${STLPORT_LIB_DIR}

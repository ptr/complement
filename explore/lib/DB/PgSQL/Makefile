# -*- Makefile -*- Time-stamp: <03/09/25 12:02:31 ptr>
# $Id$

SRCROOT := ../../..
COMPILER_NAME := gcc
PG_INCLUDE := /opt/pgsql/include
PG_LIB := /opt/pgsql/lib

include Makefile.inc
include ${SRCROOT}/Makefiles/top.mak

INCLUDES += -I$(SRCROOT)/include -I$(STLPORT_INCLUDE_DIR) -I${PG_INCLUDE}

LDSEARCH += -L${PG_LIB}
LDLIBS = -lpq

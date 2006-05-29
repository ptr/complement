# -*- Makefile -*- Time-stamp: <06/05/30 00:15:06 ptr>

SRCROOT := ../../..
COMPILER_NAME := gcc
# MYSQL_INCLUDE := /usr/include
# MYSQL_LIB := /usr/lib/lib

include Makefile.inc
include ${SRCROOT}/Makefiles/top.mak

INCLUDES += -I$(SRCROOT)/include

# LDSEARCH += -L${PG_LIB}
LDLIBS = -lmysqlclient_r

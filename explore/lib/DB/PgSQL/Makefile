# -*- Makefile -*- Time-stamp: <03/09/25 12:02:31 ptr>

SRCROOT := ../../..

PG_INCLUDE := /opt/pgsql/include
PG_LIB := /opt/pgsql/lib

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

INCLUDES += -I$(SRCROOT)/include -I${PG_INCLUDE}

LDSEARCH += -L${PG_LIB}
LDLIBS = -lpq

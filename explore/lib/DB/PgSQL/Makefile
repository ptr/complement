# -*- Makefile -*- Time-stamp: <08/06/12 15:09:05 ptr>

SRCROOT := ../../..

# PG_INCLUDE := /opt/pgsql/include
# PG_LIB_DIR := /opt/pgsql/lib

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

INCLUDES += -I$(SRCROOT)/include

ifneq (${PG_INCLUDE},)
INCLUDES += -I${PG_INCLUDE}
endif

ifneq (${PG_LIB_DIR},)
LDFLAGS += -L${PG_LIB_DIR}
endif

LDLIBS = -lpq

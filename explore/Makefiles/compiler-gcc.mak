# Time-stamp: <03/07/06 19:42:26 ptr>
# $Id$

INCLUDES = -I$(BASEDIR)/include

CXX := c++
CC := gcc

DEFS ?=
OPT ?=

OUTPUT_OPTION := -o $@
LINK_OUTPUT_OPTION := ${OUTPUT_OPTION}
CPPFLAGS = $(DEFS) $(INCLUDES)

ifeq ($(OSNAME),sunos)
CCFLAGS = -pthreads $(OPT)
CFLAGS = -pthreads $(OPT)
# CXXFLAGS = -pthreads -nostdinc++ -fexceptions -fident $(OPT)
CXXFLAGS = -pthreads  -fexceptions -fident $(OPT)
endif

ifeq ($(OSNAME),linux)
CCFLAGS = -pthread $(OPT)
CFLAGS = -pthread $(OPT)
# CXXFLAGS = -pthread -nostdinc++ -fexceptions -fident $(OPT)
CXXFLAGS = -pthread -fexceptions -fident $(OPT)
endif

CDEPFLAGS = -E -M
CCDEPFLAGS = -E -M

stldbg-static stldbg-shared : DEFS += -D_STLP_DEBUG
stldbg-static-dep stldbg-shared-dep : DEFS += -D_STLP_DEBUG

release-static release-shared : OPT += -O2

dbg-static dbg-shared : OPT += -g
dbg-static-dep dbg-shared-dep : OPT += -g

stldbg-static stldbg-shared : OPT += -g
stldbg-static-dep stldbg-shared-dep : OPT += -g

# Time-stamp: <03/07/11 14:23:27 ptr>
# $Id$

#INCLUDES = -I$(SRCROOT)/include
INCLUDES :=

CXX := cl.exe
CC := cl.exe
LINK := link.exe

DEFS ?=
OPT ?=

OUTPUT_OPTION = /Fo$@
LINK_OUTPUT_OPTION = /OUT:$@
DEFS += /D "WIN32" /D "_WINDOWS"
CPPFLAGS = $(DEFS) $(INCLUDES)

CCFLAGS = -pthread $(OPT)
CFLAGS = -pthread $(OPT)
CXXFLAGS = -pthread -fexceptions -fident $(OPT)

CFLAGS = /nologo /TC /W3 /GR /GX $(OPT)
CXXFLAGS = /nologo /TP /W3 /GR /GX $(OPT)
COMPILE.c = $(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) /c
COMPILE.cc = $(CXX) $(CXXFLAGS) $(CPPFLAGS) $(TARGET_ARCH) /c
LINK.cc = $(LINK) /nologo $(LDFLAGS) $(TARGET_ARCH)

CDEPFLAGS = /FD /E
CCDEPFLAGS = /FD /E

# STLport DEBUG mode specific defines
stldbg-static :	    DEFS += /D_DEBUG /D "_STLP_DEBUG"
stldbg-shared :     DEFS += /D_DEBUG /D "_STLP_DEBUG"
stldbg-static-dep : DEFS += /D_DEBUG /D "_STLP_DEBUG"
stldbg-shared-dep : DEFS += /D_DEBUG /D "_STLP_DEBUG"

dbg-static :	 DEFS += /D_DEBUG
dbg-shared :     DEFS += /D_DEBUG
dbg-static-dep : DEFS += /D_DEBUG
dbg-shared-dep : DEFS += /D_DEBUG

release-static :	 DEFS += /DNDEBUG
release-shared :     DEFS += /DNDEBUG
release-static-dep : DEFS += /DNDEBUG
release-shared-dep : DEFS += /DNDEBUG

# optimization and debug compiler flags
release-static : OPT += /O2 /Og
release-shared : OPT += /O2 /Og

dbg-static : OPT += /Zi
dbg-shared : OPT += /Zi
#dbg-static-dep : OPT += -g
#dbg-shared-dep : OPT += -g

stldbg-static : OPT += /Zi
stldbg-shared : OPT += /Zi
#stldbg-static-dep : OPT += -g
#stldbg-shared-dep : OPT += -g

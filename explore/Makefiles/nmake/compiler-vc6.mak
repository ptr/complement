# Time-stamp: <03/07/11 14:23:27 ptr>
# $Id$

#INCLUDES = -I$(SRCROOT)/include
INCLUDES =

CXX = cl.exe
CC = cl.exe
LINK = link.exe

!ifndef DEFS
DEFS =
!endif
!ifndef OPT
OPT =
!endif

OUTPUT_OPTION = /Fo$@
LINK_OUTPUT_OPTION = /OUT:$@
DEFS = $(DEFS) /D "WIN32" /D "_WINDOWS"
CPPFLAGS = $(DEFS) $(INCLUDES)

CFLAGS = /nologo /TC /W3 /GR /GX $(OPT)
CXXFLAGS = /nologo /TP /W3 /GR /GX $(OPT)
COMPILE_c = $(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) /c
COMPILE_cc = $(CXX) $(CXXFLAGS) $(CPPFLAGS) $(TARGET_ARCH) /c
LINK_cc = $(LINK) /nologo $(LDFLAGS) $(TARGET_ARCH)

CDEPFLAGS = /FD /E
CCDEPFLAGS = /FD /E

# STLport DEBUG mode specific defines
DEFS_STLDBG = $(DEFS) /D_DEBUG /D "_STLP_DEBUG"
DEFS_DBG = $(DEFS) /D_DEBUG
DEFS_REL = $(DEFS) /DNDEBUG

# optimization and debug compiler flags
OPT_REL = $(OPT) /O2 /Og
OPT_DBG = $(OPT) /Zi
OPT_STLDBG = $(OPT) /Zi

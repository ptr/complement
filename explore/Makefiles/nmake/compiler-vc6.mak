# Time-stamp: <03/09/15 14:20:37 ptr>
# $Id$

#INCLUDES = -I$(SRCROOT)/include
#INCLUDES =

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
DEFS_REL = $(DEFS_REL) /D "WIN32" /D "_WINDOWS"
CPPFLAGS_REL = $(DEFS_REL) $(INCLUDES)

CFLAGS_REL = /nologo /TC /W3 /GR /GX $(OPT_REL)
CXXFLAGS_REL = /nologo /TP /W3 /GR /GX $(OPT_REL)
COMPILE_c = $(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) /c
COMPILE_cc_REL = $(CXX) $(CXXFLAGS_REL) $(CPPFLAGS_REL) $(TARGET_ARCH) /c
LINK_cc_REL = $(LINK) /nologo $(LDFLAGS_REL) $(TARGET_ARCH)

CDEPFLAGS = /FD /E
CCDEPFLAGS = /FD /E

# STLport DEBUG mode specific defines
DEFS_STLDBG = $(DEFS) /D_DEBUG /D "_STLP_DEBUG"
DEFS_DBG = $(DEFS_REL) /D_DEBUG
DEFS_REL = $(DEFS_REL) /DNDEBUG

# optimization and debug compiler flags
OPT_REL = $(OPT_REL) /O2 /Og
OPT_DBG = $(OPT_REL) /Zi
OPT_STLDBG = $(OPT_REL) /Zi

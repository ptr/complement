# Time-stamp: <04/03/31 07:56:16 ptr>
# $Id$

#INCLUDES = -I$(SRCROOT)/include
#INCLUDES =

CXX = cl.exe
CC = cl.exe
LINK = link.exe
RC = rc.exe

!ifdef DEFS
DEFS_REL = $(DEFS) $(DEFS_REL)
DEFS_DBG = $(DEFS) $(DEFS_DBG)
DEFS_STLDBG = $(DEFS) $(DEFS_STLDBG)
DEFS_STATIC_REL = $(DEFS) $(DEFS_STATIC_REL)
DEFS_STATIC_DBG = $(DEFS) $(DEFS_STATIC_DBG)
DEFS_STATIC_STLDBG = $(DEFS) $(DEFS_STATIC_STLDBG)
!endif
!ifdef OPT
OPT_REL = $(OPT) $(OPT_REL)
OPT_DBG = $(OPT) $(OPT_DBG)
OPT_STLDBG = $(OPT) $(OPT_STLDBG)
OPT_STATIC_REL = $(OPT) $(OPT_STATIC_REL)
OPT_STATIC_DBG = $(OPT) $(OPT_STATIC_DBG)
OPT_STATIC_STLDBG = $(OPT) $(OPT_STATIC_STLDBG)
!endif

OUTPUT_OPTION = /Fo$@
# The sentence below is bit strange: ...$(PRGNAME)$(LIBNAME).pdb
# In this place I don't know what I build build: library or application.
# But in real projects only one will be defined---either $(PRGNAME) or
# $(LIBNAME), so .pdb name will be correct.
OUTPUT_OPTION_DBG = /Fo$@ /Fd"$(OUTPUT_DIR_DBG)/$(PRGNAME)$(LIBNAME).pdb"
OUTPUT_OPTION_STATIC_DBG = /Fo$@ /Fd"$(OUTPUT_DIR_A_DBG)/$(PRGNAME)$(LIBNAME).pdb"
OUTPUT_OPTION_STLDBG = /Fo$@ /Fd"$(OUTPUT_DIR_STLDBG)/$(PRGNAME)$(LIBNAME).pdb"
OUTPUT_OPTION_STATIC_STLDBG = /Fo$@ /Fd"$(OUTPUT_DIR_A_STLDBG)/$(PRGNAME)$(LIBNAME).pdb"
LINK_OUTPUT_OPTION = /OUT:$@
RC_OUTPUT_OPTION = /fo $@
RC_OUTPUT_OPTION_DBG = /fo $@
RC_OUTPUT_OPTION_STLDBG = /fo $@
DEFS_COMMON = /D _WIN32_WCE=$(CEVERSION) /D "$(PLATFORM)" /D UNDER_CE=$(CEVERSION) /D "UNICODE" /D "_i386_" /D "_X86_" /D "x86"
DEFS_REL = $(DEFS_REL) $(DEFS_COMMON)
DEFS_STATIC_REL = $(DEFS_STATIC_REL) $(DEFS_COMMON)
DEFS_DBG = $(DEFS_DBG) $(DEFS_COMMON)
DEFS_STATIC_DBG = $(DEFS_STATIC_DBG) $(DEFS_COMMON)
DEFS_STLDBG = $(DEFS_STLDBG) $(DEFS_COMMON)
DEFS_STATIC_STLDBG = $(DEFS_STATIC_STLDBG) $(DEFS_COMMON)
CPPFLAGS_REL = $(DEFS_REL) $(INCLUDES)
CPPFLAGS_STATIC_REL = $(DEFS_STATIC_REL) $(INCLUDES)
CPPFLAGS_DBG = $(DEFS_DBG) $(INCLUDES)
CPPFLAGS_STATIC_DBG = $(DEFS_STATIC_DBG) $(INCLUDES)
CPPFLAGS_STLDBG = $(DEFS_STLDBG) $(INCLUDES)
CPPFLAGS_STATIC_STLDBG = $(DEFS_STATIC_STLDBG) $(INCLUDES)

# exception handling support
CFLAGS_REL = /nologo /TC /W3 /GR /GX $(OPT_REL)
CFLAGS_STATIC_REL = /nologo /TC /W3 /GR /GX $(OPT_STATIC_REL)
CFLAGS_DBG = /nologo /TC /W3 /GR /GX $(OPT_DBG)
CFLAGS_STATIC_DBG = /nologo /TC /W3 /GR /GX $(OPT_STATIC_DBG)
CFLAGS_STLDBG = /nologo /TC /W3 /GR /GX $(OPT_STLDBG)
CFLAGS_STATIC_STLDBG = /nologo /TC /W3 /GR /GX $(OPT_STATIC_STLDBG)
CXXFLAGS_REL = /nologo /TP /W3 /GR /GX $(OPT_REL)
CXXFLAGS_STATIC_REL = /nologo /TP /W3 /GR /GX $(OPT_STATIC_REL)
CXXFLAGS_DBG = /nologo /TP /W3 /GR /GX $(OPT_DBG)
CXXFLAGS_STATIC_DBG = /nologo /TP /W3 /GR /GX $(OPT_STATIC_DBG)
CXXFLAGS_STLDBG = /nologo /TP /W3 /GR /GX $(OPT_STLDBG)
CXXFLAGS_STATIC_STLDBG = /nologo /TP /W3 /GR /GX $(OPT_STATIC_STLDBG)

#wihtout exceptions
#CFLAGS_REL = /nologo /TC /W3 $(OPT_REL)
#CFLAGS_STATIC_REL = /nologo /TC /W3 $(OPT_STATIC_REL)
#CFLAGS_DBG = /nologo /TC /W3 $(OPT_DBG)
#CFLAGS_STATIC_DBG = /nologo /TC /W3 $(OPT_STATIC_DBG)
#CFLAGS_STLDBG = /nologo /TC /W3 $(OPT_STLDBG)
#CFLAGS_STATIC_STLDBG = /nologo /TC /W3 $(OPT_STATIC_STLDBG)
#CXXFLAGS_REL = /nologo /TP /W3 $(OPT_REL)
#CXXFLAGS_STATIC_REL = /nologo /TP /W3 $(OPT_STATIC_REL)
#CXXFLAGS_DBG = /nologo /TP /W3 $(OPT_DBG)
#CXXFLAGS_STATIC_DBG = /nologo /TP /W3 $(OPT_STATIC_DBG)
#CXXFLAGS_STLDBG = /nologo /TP /W3 $(OPT_STLDBG)
#CXXFLAGS_STATIC_STLDBG = /nologo /TP /W3 $(OPT_STATIC_STLDBG)


COMPILE_c = $(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) /c
COMPILE_c_REL = $(CC) $(CFLAGS_REL) $(CPPFLAGS_REL) $(TARGET_ARCH) /c
COMPILE_c_STATIC_REL = $(CC) $(CFLAGS_STATIC_REL) $(CPPFLAGS_STATIC_REL) $(TARGET_ARCH) /c
COMPILE_c_DBG = $(CC) $(CFLAGS_DBG) $(CPPFLAGS_DBG) $(TARGET_ARCH) /c
COMPILE_c_STATIC_DBG = $(CC) $(CFLAGS_STATIC_DBG) $(CPPFLAGS_STATIC_DBG) $(TARGET_ARCH) /c
COMPILE_cc_REL = $(CXX) $(CXXFLAGS_REL) $(CPPFLAGS_REL) $(TARGET_ARCH) /c
COMPILE_cc_STATIC_REL = $(CXX) $(CXXFLAGS_STATIC_REL) $(CPPFLAGS_STATIC_REL) $(TARGET_ARCH) /c
COMPILE_cc_DBG = $(CXX) $(CXXFLAGS_DBG) $(CPPFLAGS_DBG) $(TARGET_ARCH) /c
COMPILE_cc_STATIC_DBG = $(CXX) $(CXXFLAGS_STATIC_DBG) $(CPPFLAGS_STATIC_DBG) $(TARGET_ARCH) /c
COMPILE_cc_STLDBG = $(CXX) $(CXXFLAGS_STLDBG) $(CPPFLAGS_STLDBG) $(TARGET_ARCH) /c
COMPILE_cc_STATIC_STLDBG = $(CXX) $(CXXFLAGS_STATIC_STLDBG) $(CPPFLAGS_STATIC_STLDBG) $(TARGET_ARCH) /c
COMPILE_rc_REL = $(RC) $(RC_FLAGS_REL)
COMPILE_rc_STATIC_REL = $(RC) $(RC_FLAGS_REL)
COMPILE_rc_DBG = $(RC) $(RC_FLAGS_DBG)
COMPILE_rc_STATIC_DBG = $(RC) $(RC_FLAGS_DBG)
COMPILE_rc_STLDBG = $(RC) $(RC_FLAGS_STLDBG)
COMPILE_rc_STATIC_STLDBG = $(RC) $(RC_FLAGS_STLDBG)
LINK_cc_REL = $(LINK) /nologo $(LDFLAGS_REL)
LINK_cc_DBG = $(LINK) /nologo /debug /pdb:"$(OUTPUT_DIR_DBG)/$(PRGNAME)$(LIBNAME).pdb" $(LDFLAGS_DBG)
LINK_cc_STLDBG = $(LINK) /nologo /debug /pdb:"$(OUTPUT_DIR_STLDBG)/$(PRGNAME)$(LIBNAME).pdb" $(LDFLAGS_STLDBG)

CDEPFLAGS = /FD /E
CCDEPFLAGS = /FD /E

# STLport DEBUG mode specific defines
DEFS_STLDBG = $(DEFS_STLDBG) /D_DEBUG /D "_STLP_DEBUG" /DDEBUG 
DEFS_DBG = $(DEFS_DBG) /D_DEBUG /DDEBUG
DEFS_REL = $(DEFS_REL) /DNDEBUG
DEFS_STATIC_STLDBG = $(DEFS_STATIC_STLDBG) /D_DEBUG /D "_STLP_DEBUG" /DDEBUG /D "_STLP_NO_FORCE_INSTANTIATE"
DEFS_STATIC_DBG = $(DEFS_STATIC_DBG) /D_DEBUG /DDEBUG /D "_STLP_NO_FORCE_INSTANTIATE"
DEFS_STATIC_REL = $(DEFS_STATIC_REL) /DNDEBUG /D "_STLP_NO_FORCE_INSTANTIATE"

# optimization and debug compiler flags
OPT_COMMON = /Gs8192 /GF
OPT_REL = $(OPT_REL) /O2 /Og $(OPT_COMMON)
OPT_DBG = $(OPT_DBG) /Zi /Od $(OPT_COMMON)
OPT_STLDBG = $(OPT_STLDBG) /Zi /Od $(OPT_COMMON)
OPT_STATIC_REL = $(OPT_STATIC_REL) /O2 /Og $(OPT_COMMON)
OPT_STATIC_DBG = $(OPT_STATIC_DBG) /Zi /Od $(OPT_COMMON)
OPT_STATIC_STLDBG = $(OPT_STATIC_STLDBG) /Zi /Od $(OPT_COMMON)

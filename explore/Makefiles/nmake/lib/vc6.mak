# -*- makefile -*- Time-stamp: <03/09/28 16:39:38 ptr>
# $Id$


# Oh, the commented below work for gmake 3.78.1 and above,
# but phrase without tag not work for it. Since gmake 3.79 
# tag with assignment fail, but work assignment for all tags
# (really that more correct).

!ifndef LDLIBS
LDLIBS =
!endif

LDSEARCH = $(LDSEARCH) /LIBPATH:"$(MSVC_LIB_DIR)"

OPT_DBG = $(OPT_DBG) /MDd
OPT_STLDBG = $(OPT_STLDBG) /MDd
OPT_REL = $(OPT_REL) /MD
OPT_DBG_STATIC = $(OPT_DBG_STATIC) /MTd
OPT_STLDBG_STATIC = $(OPT_STLDBG_STATIC) /MTd
OPT_REL_STATIC = $(OPT_REL_STATIC) /MT

LDFLAGS_REL = $(LDFLAGS_REL) /DLL $(LDSEARCH)
LDFLAGS_DBG = $(LDFLAGS_DBG) /DLL $(LDSEARCH)
LDFLAGS_STLDBG = $(LDFLAGS_STLDBG) /DLL $(LDSEARCH)
# LDFLAGS_STATIC = $(LDSEARCH)

LDFLAGS_REL = $(LDFLAGS_REL) /VERSION:$(MAJOR).$(MINOR)
LDFLAGS_DBG = $(LDFLAGS_DBG) /VERSION:$(MAJOR).$(MINOR)
LDFLAGS_STLDBG = $(LDFLAGS_STLDBG) /VERSION:$(MAJOR).$(MINOR)

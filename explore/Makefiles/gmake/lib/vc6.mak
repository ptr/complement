# -*- makefile -*- Time-stamp: <08/06/12 15:03:15 ptr>
#
# Copyright (c) 1997-1999, 2002, 2003, 2005-2008
# Petr Ovtchenkov
#
# Portion Copyright (c) 1999-2001
# Parallel Graphics Ltd.
#
# Licensed under the Academic Free License version 3.0
#

LDLIBS ?=

dbg-shared:	OPT += /MDd
stldbg-shared:	OPT += /MDd
release-shared:	OPT += /MD
release-shared-dep:	OPT += /MD
dbg-static:	OPT += /MTd
stldbg-static:	OPT += /MTd
release-static:	OPT += /MT

release-static:	DEFS += /D_LIB
dbg-static:	DEFS += /D_LIB
stldbg-static:	DEFS += /D_LIB


dbg-shared:	LDFLAGS += /DLL
stldbg-shared:	LDFLAGS += /DLL
release-shared:	LDFLAGS += /DLL

LDFLAGS +=  /LIBPATH:"$(MSVC_LIB_DIR)" /VERSION:$(MAJOR).$(MINOR)

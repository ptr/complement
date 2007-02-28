# -*- makefile -*- Time-stamp: <03/10/10 16:15:53 ptr>
#
# Copyright (c) 1997-1999, 2002, 2003, 2005, 2006
# Petr Ovtchenkov
#
# Portion Copyright (c) 1999-2001
# Parallel Graphics Ltd.
#
# Licensed under the Academic Free License version 3.0
#

LDFLAGS ?= 

ifneq ("$(findstring $(OSNAME),darwin windows)","")
include ${RULESBASE}/${USE_MAKE}/${OSNAME}/lib.mak
else
include ${RULESBASE}/${USE_MAKE}/unix/lib.mak
endif

include ${RULESBASE}/${USE_MAKE}/lib/${COMPILER_NAME}.mak

ifneq ("$(findstring $(OSNAME),windows)","")
include ${RULESBASE}/${USE_MAKE}/${OSNAME}/rules-so.mak
else
include ${RULESBASE}/${USE_MAKE}/unix/rules-so.mak
endif

include ${RULESBASE}/${USE_MAKE}/lib/rules-a.mak

ifneq ("$(findstring $(OSNAME),windows)","")
include ${RULESBASE}/${USE_MAKE}/${OSNAME}/rules-install-so.mak
else
include ${RULESBASE}/${USE_MAKE}/unix/rules-install-so.mak
endif

include ${RULESBASE}/${USE_MAKE}/lib/rules-install-a.mak
include ${RULESBASE}/${USE_MAKE}/lib/clean.mak

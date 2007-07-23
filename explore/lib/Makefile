# Time-stamp: <07/01/23 14:12:04 ptr>
#
# Copyright (c) 2006, 2007
# Petr Ovtchenkov
#
# Licensed under the Academic Free License version 3.0
#

SRCROOT := ..
SUBDIRS := exam mt sockios stem

include ${SRCROOT}/Makefiles/gmake/subdirs.mak

all install depend clean clobber distclean check:
	+$(call doinsubdirs,${SUBDIRS})

.PHONY: all install depend clean clobber distclean check

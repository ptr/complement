# Time-stamp: <08/06/30 12:51:10 yeti>
#
# Copyright (c) 2006-2008
# Petr Ovtchenkov
#
# Licensed under the Academic Free License version 3.0
#

SRCROOT := ..
SUBDIRS := misc exam mt sockios stem

include ${SRCROOT}/Makefiles/gmake/subdirs.mak

all install depend clean clobber distclean check:
	+$(call doinsubdirs,${SUBDIRS})

.PHONY: all install depend clean clobber distclean check

# Time-stamp: <08/06/30 12:51:10 yeti>
#
# Copyright (c) 2006-2008
# Petr Ovtchenkov
#
# Licensed under the Academic Free License version 3.0
#

SRCROOT := ..
SUBDIRS := misc exam mt sockios stem

-include ${SRCROOT}/Makefiles/gmake/config.mak
include ${SRCROOT}/Makefiles/gmake/subdirs.mak

ifndef WITHOUT_STLPORT
STLP_TAGS = install-stldbg-shared stldbg-shared check-stldbg-shared
endif

all install install-release-shared install-dbg-shared depend clean distclean maintainer-clean mostlyclean check check-release-shared check-dbg-shared release-shared dbg-shared ${STLP_TAGS}:
	+$(call doinsubdirs,${SUBDIRS})

.PHONY: all install install-release-shared install-dbg-shared depend clean distclean distclean maintainer-clean mostlyclean check check-release-shared check-dbg-shared release-shared dbg-shared ${STLP_TAGS}

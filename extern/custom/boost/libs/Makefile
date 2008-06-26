# -*- makefile -*- Time-stamp: <07/02/03 09:26:04 ptr>
#
# Copyright (c) 2006, 2007
# Petr Ovtchenkov
#
# Licensed under the Academic Free License version 3.0
#

SRCROOT := ../../../../explore
SUBDIRS := date_time filesystem program_options regex serialization \
           test/unit_test_framework test/prg_exec_mon \
           thread

include ${SRCROOT}/Makefiles/gmake/subdirs.mak

all install install-release-static depend clean clobber distclean check:
	$(call doinsubdirs,${SUBDIRS})

.PHONY: all install install-release-static depend clean clobber distclean check

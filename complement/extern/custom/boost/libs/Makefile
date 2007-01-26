# -*- makefile -*- Time-stamp: <06/11/01 22:56:34 ptr>
#
# Copyright (c) 2006
# Petr Ovtchenkov
#
# Licensed under the Academic Free License version 3.0
#

SRCROOT := ../../../../explore
SUBDIRS := date_time filesystem program_options regex serialization \
           test/unit_test_framework test/test_exec_mon test/prg_exec_mon \
           thread

include ${SRCROOT}/Makefiles/gmake/subdirs.mak

all install install-release-static depend clean clobber distclean check:
	$(doinsubdirs)

.PHONY: all install install-release-static depend clean clobber distclean check

# -*- makefile -*- Time-stamp: <09/10/05 23:06:58 ptr>
#
# Copyright (c) 2006, 2007, 2009, 2010
# Petr Ovtchenkov
#
# Licensed under the Academic Free License version 3.0
#

SRCROOT := ../../../../explore
SUBDIRS := date_time filesystem program_options regex \
           test/unit_test_framework test/prg_exec_mon \
           thread python system log graph

include ${SRCROOT}/Makefiles/gmake/subdirs.mak

all install install-release-static depend clean clobber distclean check:
	$(call doinsubdirs,${SUBDIRS})

.PHONY: all install install-release-static depend clean clobber distclean check

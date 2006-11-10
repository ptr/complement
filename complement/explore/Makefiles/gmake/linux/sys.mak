# Time-stamp: <06/11/10 19:18:10 ptr>
#
# Copyright (c) 1997-1999, 2002, 2003, 2005, 2006
# Petr Ovtchenkov
#
# Portion Copyright (c) 1999-2001
# Parallel Graphics Ltd.
#
# Licensed under the Academic Free License version 3.0
#

INSTALL := /usr/bin/install

STRIP := /usr/bin/strip

install-strip:	_INSTALL_STRIP_OPTION = -s

INSTALL_SO := ${INSTALL} -c -m 0755 ${_INSTALL_STRIP_OPTION}
INSTALL_A := ${INSTALL} -c -m 0644
INSTALL_EXE := ${INSTALL} -c -m 0755

# bash's built-in test is like extern
# EXT_TEST := /usr/bin/test
EXT_TEST := test

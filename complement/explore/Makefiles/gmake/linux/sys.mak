# Time-stamp: <05/09/09 21:01:23 ptr>
# $Id$

INSTALL := /usr/bin/install

INSTALL_SO := ${INSTALL} -c -m 0755
INSTALL_A := ${INSTALL} -c -m 0644
INSTALL_EXE := ${INSTALL} -c -m 0755

# bash's built-in test is like extern
# EXT_TEST := /usr/bin/test
EXT_TEST := test

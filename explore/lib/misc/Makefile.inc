# -*- Makefile -*- Time-stamp: <02/08/01 09:27:30 ptr>
# $Id$

# I have only one reason while I should use "C++" variant of MD5 instead of "C":
# names like MD5Init is wide distributed, but some cool programmers use this
# name for something different from RFC1321, and other advanced programmers
# (like HP's) write linkers that take local (!!!) symbols from other module
# instead of global one. So I should shift this module in other namespace
# that possible for C++ (or I should #define names).

LIBNAME = misc
MAJOR = .1
MINOR = .8
PATCH = .1
SRC_CC = CyrMoney.cc args.cc dir_utils.cc directory.cc _node.cc arguments.cc
SRC_C = md5.c

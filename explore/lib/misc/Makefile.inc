# Time-stamp: <00/10/16 14:57:00 ptr>
# $SunId$

# I have only one reason while I should use "C++" variant of MD5 instead of "C":
# names like MD5Init is wide distributed, but some cool programmers use this
# name for something different from RFC1321, and other advanced programmers
# (like HP's) write linkers that take local (!!!) symbols from other module
# instead of global one. So I should shift this module in other namespace
# that possible for C++ (or I should #define names).

LIBNAME = Aux
MINOR = .2
SRC_CC = CyrMoney.cc args.cc dir_utils.cc directory.cc _node.cc cgi.cc # _md5.cc
SRC_C = md5.c

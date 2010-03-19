# -*- makefile -*- Time-stamp: <10/03/18 09:37:48 ptr>

LIBNAME = sigc++
MAJOR = 2
MINOR = 2
PATCH = 4.2
SRC_CC = ${LIBSIGCXX_SRC}/signal.cc                \
         ${LIBSIGCXX_SRC}/signal_base.cc           \
         ${LIBSIGCXX_SRC}/trackable.cc             \
         ${LIBSIGCXX_SRC}/connection.cc            \
         ${LIBSIGCXX_SRC}/functors/slot.cc         \
         ${LIBSIGCXX_SRC}/functors/slot_base.cc    \
         adaptors/lambda/lambda.cc


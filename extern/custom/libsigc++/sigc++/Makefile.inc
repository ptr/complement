# -*- makefile -*- Time-stamp: <10/03/22 14:31:51 ptr>

LIBSIGCXX_SRC = ${LIBSIGCXX_DIR}/sigc++

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


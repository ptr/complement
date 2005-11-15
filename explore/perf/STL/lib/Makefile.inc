# -*- Makefile -*- Time-stamp: <04/03/25 23:18:30 ptr>
# $Id$

LIBNAME = stlport
MAJOR = 5
MINOR = 0
PATCH = 0
SRC_CPP = ${STLPORT_SRC_DIR}/src/dll_main.cpp \
          ${STLPORT_SRC_DIR}/src/fstream.cpp \
          ${STLPORT_SRC_DIR}/src/strstream.cpp \
          ${STLPORT_SRC_DIR}/src/sstream.cpp \
          ${STLPORT_SRC_DIR}/src/ios.cpp \
          ${STLPORT_SRC_DIR}/src/stdio_streambuf.cpp \
          ${STLPORT_SRC_DIR}/src/istream.cpp \
          ${STLPORT_SRC_DIR}/src/ostream.cpp \
          ${STLPORT_SRC_DIR}/src/iostream.cpp \
          ${STLPORT_SRC_DIR}/src/codecvt.cpp \
          ${STLPORT_SRC_DIR}/src/collate.cpp \
          ${STLPORT_SRC_DIR}/src/ctype.cpp \
          ${STLPORT_SRC_DIR}/src/monetary.cpp \
          ${STLPORT_SRC_DIR}/src/num_get.cpp \
          ${STLPORT_SRC_DIR}/src/num_put.cpp \
          ${STLPORT_SRC_DIR}/src/num_get_float.cpp \
          ${STLPORT_SRC_DIR}/src/num_put_float.cpp \
          ${STLPORT_SRC_DIR}/src/numpunct.cpp \
          ${STLPORT_SRC_DIR}/src/time_facets.cpp \
          ${STLPORT_SRC_DIR}/src/messages.cpp \
          ${STLPORT_SRC_DIR}/src/locale_impl.cpp \
          ${STLPORT_SRC_DIR}/src/locale_catalog.cpp \
          ${STLPORT_SRC_DIR}/src/facets_byname.cpp \
          ${STLPORT_SRC_DIR}/src/complex.cpp \
          ${STLPORT_SRC_DIR}/src/complex_exp.cpp \
          ${STLPORT_SRC_DIR}/src/complex_io.cpp \
          ${STLPORT_SRC_DIR}/src/complex_trig.cpp \
          ${STLPORT_SRC_DIR}/src/complex_io_w.cpp \
          ${STLPORT_SRC_DIR}/src/string_w.cpp

SRC_C = ${STLPORT_SRC_DIR}/src/c_locale.c

SRC_RC = ${STLPORT_SRC_DIR}/src/stlport.rc

# -*- Makefile -*- Time-stamp: <04/03/25 23:18:30 ptr>
# $Id$

LIBNAME = stlport
MAJOR = 5
MINOR = 0
PATCH = 0
SRC_CPP = ${SRCROOT}/../dll_main.cpp \
          ${SRCROOT}/../fstream.cpp \
          ${SRCROOT}/../strstream.cpp \
          ${SRCROOT}/../sstream.cpp \
          ${SRCROOT}/../ios.cpp \
          ${SRCROOT}/../stdio_streambuf.cpp \
          ${SRCROOT}/../istream.cpp \
          ${SRCROOT}/../ostream.cpp \
          ${SRCROOT}/../iostream.cpp \
          ${SRCROOT}/../codecvt.cpp \
          ${SRCROOT}/../collate.cpp \
          ${SRCROOT}/../ctype.cpp \
          ${SRCROOT}/../monetary.cpp \
          ${SRCROOT}/../num_get.cpp \
          ${SRCROOT}/../num_put.cpp \
          ${SRCROOT}/../num_get_float.cpp \
          ${SRCROOT}/../num_put_float.cpp \
          ${SRCROOT}/../numpunct.cpp \
          ${SRCROOT}/../time_facets.cpp \
          ${SRCROOT}/../messages.cpp \
          ${SRCROOT}/../locale_impl.cpp \
          ${SRCROOT}/../locale.cpp \
          ${SRCROOT}/../locale_catalog.cpp \
          ${SRCROOT}/../facets_byname.cpp \
          ${SRCROOT}/../c_locale_stub.cpp \
          ${SRCROOT}/../complex.cpp \
          ${SRCROOT}/../complex_exp.cpp \
          ${SRCROOT}/../complex_io.cpp \
          ${SRCROOT}/../complex_trig.cpp \
          ${SRCROOT}/../complex_io_w.cpp \
          ${SRCROOT}/../string_w.cpp

SRC_C = ${SRCROOT}/../c_locale.c

SRC_RC = ${SRCROOT}/../stlport.rc

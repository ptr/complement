# -*- makefile -*- Time-stamp: <09/07/17 23:35:37 ptr>

BOOST_SR_SRC = ${BOOST_INCLUDE_DIR}/libs/program_options/src

LIBNAME = boost_program_options
MAJOR = 1
MINOR = 39
PATCH = 0

SRC_CPP = ${BOOST_SR_SRC}/cmdline.cpp \
          ${BOOST_SR_SRC}/config_file.cpp \
          ${BOOST_SR_SRC}/options_description.cpp \
          ${BOOST_SR_SRC}/parsers.cpp \
          ${BOOST_SR_SRC}/variables_map.cpp \
          ${BOOST_SR_SRC}/value_semantic.cpp \
          ${BOOST_SR_SRC}/positional_options.cpp \
          ${BOOST_SR_SRC}/utf8_codecvt_facet.cpp \
          ${BOOST_SR_SRC}/convert.cpp \
          ${BOOST_SR_SRC}/winmain.cpp

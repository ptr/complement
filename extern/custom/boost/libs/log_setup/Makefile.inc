# -*- makefile -*- Time-stamp: <10/03/15 22:58:42 ptr>

BOOST_S_SRC = ${BOOST_INCLUDE_DIR}/libs/system/src
BOOST_LOG_SRC_DIR = ${BOOST_LOG_DIR}/libs/log/src
LIBNAME = boost_log_setup
MAJOR = 1
MINOR = 43
PATCH = 0
SRC_CPP = ${BOOST_LOG_SRC_DIR}/parser_utils.cpp \
          ${BOOST_LOG_SRC_DIR}/init_from_stream.cpp \
          ${BOOST_LOG_SRC_DIR}/init_from_settings.cpp \
          ${BOOST_LOG_SRC_DIR}/filter_parser.cpp \
          ${BOOST_LOG_SRC_DIR}/formatter_parser.cpp

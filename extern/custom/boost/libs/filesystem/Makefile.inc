# -*- makefile -*- Time-stamp: <07/10/31 11:51:27 ptr>

BOOST_FS_SRC = ${BOOST_INCLUDE_DIR}/libs/filesystem/src
#BOOST_FS_SRC = src
LIBNAME = boost_fs
MAJOR = 1
MINOR = 37
PATCH = 0
SRC_CPP = ${BOOST_FS_SRC}/operations.cpp \
          ${BOOST_FS_SRC}/path.cpp \
          ${BOOST_FS_SRC}/portability.cpp \
          ${BOOST_FS_SRC}/utf8_codecvt_facet.cpp \
          ${BOOST_INCLUDE_DIR}/libs/system/src/error_code.cpp

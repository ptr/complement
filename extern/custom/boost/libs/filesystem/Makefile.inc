# -*- makefile -*- Time-stamp: <09/07/17 23:32:29 ptr>

BOOST_FS_SRC = ${BOOST_INCLUDE_DIR}/libs/filesystem
#BOOST_FS_SRC = src
LIBNAME = boost_fs
MAJOR = 1
MINOR = 44
PATCH = 0
SRC_CPP = ${BOOST_FS_SRC}/v2/src/v2_operations.cpp \
          ${BOOST_FS_SRC}/v2/src/v2_path.cpp \
          ${BOOST_FS_SRC}/v2/src/v2_portability.cpp \
          ${BOOST_FS_SRC}/v3/src/operations.cpp \
          ${BOOST_FS_SRC}/v3/src/path.cpp \
          ${BOOST_FS_SRC}/v3/src/portability.cpp \
          ${BOOST_FS_SRC}/v3/src/utf8_codecvt_facet.cpp \
          ${BOOST_FS_SRC}/v3/src/codecvt_error_category.cpp \
          ${BOOST_FS_SRC}/v3/src/path_traits.cpp \
          ${BOOST_FS_SRC}/v3/src/unique_path.cpp \
          ${BOOST_FS_SRC}/v3/src/windows_file_codecvt.cpp \
          ${BOOST_INCLUDE_DIR}/libs/system/src/error_code.cpp

#${BOOST_FS_SRC}/operations.cpp \
#          ${BOOST_FS_SRC}/path.cpp \
#          ${BOOST_FS_SRC}/portability.cpp \
#          ${BOOST_FS_SRC}/utf8_codecvt_facet.cpp \
#          ${BOOST_INCLUDE_DIR}/libs/system/src/error_code.cpp

# -*- makefile -*- Time-stamp: <03/03/31 15:11:21 ptr>

BOOST_FS_SRC = ${BOOST_INCLUDE_DIR}/libs/filesystem/src
#BOOST_FS_SRC = src
LIBNAME = boost_fs
MAJOR = 1
MINOR = 34
PATCH = 0
SRC_CPP = ${BOOST_FS_SRC}/exception.cpp ${BOOST_FS_SRC}/operations.cpp ${BOOST_FS_SRC}/path.cpp ${BOOST_FS_SRC}/portability.cpp ${BOOST_FS_SRC}/utf8_codecvt_facet.cpp

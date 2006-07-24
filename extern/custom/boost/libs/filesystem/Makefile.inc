# -*- makefile -*- Time-stamp: <03/03/31 15:11:21 ptr>

BOOST_FS_SRC = ${BOOST_INCLUDE_DIR}/libs/filesystem/src
#BOOST_FS_SRC = src
LIBNAME = boost_fs
MAJOR = 1
MINOR = 33
PATCH = 1
SRC_CPP = ${BOOST_FS_SRC}/convenience.cpp ${BOOST_FS_SRC}/exception.cpp ${BOOST_FS_SRC}/operations_posix_windows.cpp ${BOOST_FS_SRC}/path_posix_windows.cpp

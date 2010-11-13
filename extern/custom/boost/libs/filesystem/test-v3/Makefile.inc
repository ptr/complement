# -*- makefile -*- Time-stamp: <03/03/31 15:11:21 ptr>

BOOST_FS_SRC = ${BOOST_INCLUDE_DIR}/libs/filesystem/v3/test
PRGNAME = path_unit_test
PRGNAMES = path_test operations_unit_test operations_test fstream_test convenience_test
# large_file_support_test
SRC_CPP = ${BOOST_FS_SRC}/path_unit_test.cpp
path_test_SRC_CPP = ${BOOST_FS_SRC}/path_test.cpp
operations_test_SRC_CPP = ${BOOST_FS_SRC}/operations_test.cpp
operations_unit_test_SRC_CPP = ${BOOST_FS_SRC}/operations_unit_test.cpp
fstream_test_SRC_CPP = ${BOOST_FS_SRC}/fstream_test.cpp
convenience_test_SRC_CPP = ${BOOST_FS_SRC}/convenience_test.cpp
# large_file_support_test_SRC_CPP = ${BOOST_FS_SRC}/large_file_support_test.cpp

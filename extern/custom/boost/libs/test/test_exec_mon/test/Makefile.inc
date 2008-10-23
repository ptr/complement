# -*- makefile -*- Time-stamp: <07/01/24 23:23:35 ptr>

BOOST_FS_SRC = ${BOOST_INCLUDE_DIR}/libs/test/test
PRGNAME = test_exec_fail1
PRGNAMES = test_exec_fail2 test_exec_fail3 test_exec_fail4 \
           boost_check_equal_str
SRC_CPP = ${BOOST_FS_SRC}/test_exec_fail1.cpp
test_exec_fail2_SRC_CPP = ${BOOST_FS_SRC}/test_exec_fail2.cpp
test_exec_fail3_SRC_CPP = ${BOOST_FS_SRC}/test_exec_fail3.cpp
test_exec_fail4_SRC_CPP = ${BOOST_FS_SRC}/test_exec_fail4.cpp
boost_check_equal_str_SRC_CPP = ${BOOST_FS_SRC}/boost_check_equal_str.cpp

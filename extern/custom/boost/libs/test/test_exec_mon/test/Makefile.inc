# -*- makefile -*- Time-stamp: <07/01/24 23:23:35 ptr>

BOOST_TST_SRC = ${BOOST_INCLUDE_DIR}/libs/test/test
PRGNAME = prg_exec_fail1
PRGNAMES = prg_exec_fail2 prg_exec_fail3 prg_exec_fail4 \
           boost_check_equal_str
SRC_CPP = ${BOOST_TST_SRC}/prg_exec_fail1.cpp
test_exec_fail2_SRC_CPP = ${BOOST_TST_SRC}/prg_exec_fail2.cpp
test_exec_fail3_SRC_CPP = ${BOOST_TST_SRC}/prg_exec_fail3.cpp
test_exec_fail4_SRC_CPP = ${BOOST_TST_SRC}/prg_exec_fail4.cpp
boost_check_equal_str_SRC_CPP = ${BOOST_TST_SRC}/boost_check_equal_str.cpp

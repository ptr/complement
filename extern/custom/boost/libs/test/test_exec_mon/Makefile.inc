# -*- makefile -*- Time-stamp: <03/03/31 15:11:21 ptr>
# $Id$

BOOST_TST_SRC = ${BOOST_INCLUDE_DIR}/libs/test/src
LIBNAME = boost_test_tem
MAJOR = 1
MINOR = 31
PATCH = 0
SRC_CPP = ${BOOST_TST_SRC}/execution_monitor.cpp ${BOOST_TST_SRC}/test_tools.cpp \
          ${BOOST_TST_SRC}/unit_test_parameters.cpp \
          ${BOOST_TST_SRC}/unit_test_log.cpp ${BOOST_TST_SRC}/unit_test_monitor.cpp \
          ${BOOST_TST_SRC}/unit_test_result.cpp \
          ${BOOST_TST_SRC}/unit_test_suite.cpp ${BOOST_TST_SRC}/test_main.cpp \
	  ${BOOST_TST_SRC}/supplied_log_formatters.cpp

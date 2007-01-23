# -*- makefile -*- Time-stamp: <03/03/31 15:11:21 ptr>

BOOST_TST_SRC = ${BOOST_INCLUDE_DIR}/libs/test/src
LIBNAME = boost_test_tem
MAJOR = 1
MINOR = 33
PATCH = 1
SRC_CPP = ${BOOST_TST_SRC}/compiler_log_formatter.cpp \
          ${BOOST_TST_SRC}/execution_monitor.cpp \
          ${BOOST_TST_SRC}/framework.cpp \
          ${BOOST_TST_SRC}/plain_report_formatter.cpp \
          ${BOOST_TST_SRC}/progress_monitor.cpp \
          ${BOOST_TST_SRC}/results_collector.cpp \
          ${BOOST_TST_SRC}/results_reporter.cpp \
          ${BOOST_TST_SRC}/test_main.cpp \
          ${BOOST_TST_SRC}/test_tools.cpp \
          ${BOOST_TST_SRC}/unit_test_log.cpp \
          ${BOOST_TST_SRC}/unit_test_main.cpp \
          ${BOOST_TST_SRC}/unit_test_monitor.cpp \
          ${BOOST_TST_SRC}/unit_test_parameters.cpp \
          ${BOOST_TST_SRC}/unit_test_suite.cpp \
          ${BOOST_TST_SRC}/xml_log_formatter.cpp \
          ${BOOST_TST_SRC}/xml_report_formatter.cpp

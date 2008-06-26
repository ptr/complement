# -*- makefile -*- Time-stamp: <06/05/26 09:05:07 ptr>

BOOST_TST_SRC = $(BOOST_INCLUDE_DIR)/libs/test/src
LIBNAME = boost_test_utf
MAJOR = 1
MINOR = 34
PATCH = 0
SRC_CPP = $(BOOST_TST_SRC)/compiler_log_formatter.cpp \
          $(BOOST_TST_SRC)/execution_monitor.cpp \
          $(BOOST_TST_SRC)/framework.cpp \
          $(BOOST_TST_SRC)/plain_report_formatter.cpp \
          $(BOOST_TST_SRC)/progress_monitor.cpp \
          $(BOOST_TST_SRC)/results_collector.cpp \
          $(BOOST_TST_SRC)/results_reporter.cpp \
          $(BOOST_TST_SRC)/test_tools.cpp \
          $(BOOST_TST_SRC)/unit_test_log.cpp \
          $(BOOST_TST_SRC)/unit_test_main.cpp \
          $(BOOST_TST_SRC)/unit_test_monitor.cpp \
          $(BOOST_TST_SRC)/unit_test_parameters.cpp \
          $(BOOST_TST_SRC)/unit_test_suite.cpp \
          $(BOOST_TST_SRC)/xml_log_formatter.cpp \
          $(BOOST_TST_SRC)/xml_report_formatter.cpp

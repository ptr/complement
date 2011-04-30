# -*- makefile -*- Time-stamp: <2011-04-21 13:20:02 ptr>

PRGNAME = misc_sut
PRGNAMES = misc_ut opts_ut
SRC_CC = unit_test.cc \
         misc_test.cc misc_test_suite.cc opts_test.cc opts_test_suite.cc \
         chrono_test.cc
opts_ut_SRC_CC = opts_ut.cc \
         opts_test.cc opts_test_suite.cc
misc_ut_SRC_CC = misc_ut.cc \
         misc_test.cc misc_test_suite.cc \
         chrono_test.cc

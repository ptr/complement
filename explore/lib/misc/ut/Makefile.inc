# -*- makefile -*-

PRGNAME = misc_sut
PRGNAMES = misc_ut opts_ut hashes_ut
SRC_CC = unit_test.cc \
         misc_test.cc misc_test_suite.cc opts_test.cc opts_test_suite.cc \
         chrono_test.cc
opts_ut_SRC_CC = opts_ut.cc \
         opts_test.cc opts_test_suite.cc
misc_ut_SRC_CC = misc_ut.cc \
         misc_test.cc misc_test_suite.cc \
         chrono_test.cc
hashes_ut_SRC_CC = hashes_ut.cc \
         hashes_test.cc hashes_test_suite.cc

# -*- makefile -*- Time-stamp: <07/01/25 00:54:51 ptr>

BOOST_FS_SRC = ${BOOST_INCLUDE_DIR}/libs/program_options/test
PRGNAME = options_description_test
PRGNAMES = parsers_test variable_map_test cmdline_test positional_options_test \
          unicode_test winmain
SRC_CPP = ${BOOST_FS_SRC}/options_description_test.cpp
parsers_test_SRC_CPP = ${BOOST_FS_SRC}/parsers_test.cpp
variable_map_test_SRC_CPP = ${BOOST_FS_SRC}/variable_map_test.cpp
cmdline_test_SRC_CPP = ${BOOST_FS_SRC}/cmdline_test.cpp
positional_options_test_SRC_CPP = ${BOOST_FS_SRC}/positional_options_test.cpp
unicode_test_SRC_CPP = ${BOOST_FS_SRC}/unicode_test.cpp
winmain_SRC_CPP = ${BOOST_FS_SRC}/winmain.cpp

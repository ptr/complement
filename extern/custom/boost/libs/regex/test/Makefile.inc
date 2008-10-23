# -*- makefile -*- Time-stamp: <07/01/25 09:51:52 ptr>

BOOST_FS_SRC = ${BOOST_INCLUDE_DIR}/libs/regex/test
PRGNAME = regex
PRGNAMES = posix_api_check wide_posix_api_check bad_expression_test \
           recursion_test unicode_iterator_test
SRC_CPP = ${BOOST_FS_SRC}/regress/basic_tests.cpp \
          ${BOOST_FS_SRC}/regress/main.cpp \
          ${BOOST_FS_SRC}/regress/test_alt.cpp \
          ${BOOST_FS_SRC}/regress/test_anchors.cpp \
          ${BOOST_FS_SRC}/regress/test_asserts.cpp \
          ${BOOST_FS_SRC}/regress/test_backrefs.cpp \
          ${BOOST_FS_SRC}/regress/test_deprecated.cpp \
          ${BOOST_FS_SRC}/regress/test_emacs.cpp \
          ${BOOST_FS_SRC}/regress/test_escapes.cpp \
          ${BOOST_FS_SRC}/regress/test_grep.cpp \
          ${BOOST_FS_SRC}/regress/test_locale.cpp \
          ${BOOST_FS_SRC}/regress/test_mfc.cpp \
          ${BOOST_FS_SRC}/regress/test_non_greedy_repeats.cpp \
          ${BOOST_FS_SRC}/regress/test_perl_ex.cpp \
          ${BOOST_FS_SRC}/regress/test_replace.cpp \
          ${BOOST_FS_SRC}/regress/test_sets.cpp \
          ${BOOST_FS_SRC}/regress/test_simple_repeats.cpp \
          ${BOOST_FS_SRC}/regress/test_tricky_cases.cpp \
          ${BOOST_FS_SRC}/regress/test_icu.cpp \
          ${BOOST_FS_SRC}/regress/test_unicode.cpp \
          ${BOOST_FS_SRC}/regress/test_overloads.cpp \
          ${BOOST_FS_SRC}/regress/test_operators.cpp

posix_api_check_SRC_CPP = ${BOOST_FS_SRC}/c_compiler_checks/posix_api_check.cpp
wide_posix_api_check_SRC_CPP = ${BOOST_FS_SRC}/c_compiler_checks/wide_posix_api_check.cpp
bad_expression_test_SRC_CPP = ${BOOST_FS_SRC}/pathology/bad_expression_test.cpp
recursion_test_SRC_CPP = ${BOOST_FS_SRC}/pathology/recursion_test.cpp
unicode_iterator_test_SRC_CPP = ${BOOST_FS_SRC}/unicode/unicode_iterator_test.cpp

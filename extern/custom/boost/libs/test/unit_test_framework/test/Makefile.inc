# -*- makefile -*- Time-stamp: <03/03/31 15:11:21 ptr>

BOOST_FS_SRC = ${BOOST_INCLUDE_DIR}/libs/test/test
PRGNAME = class_properties_test
PRGNAMES = basic_cstring_test output_test_stream_test parameterized_test_test test_fp_comparisons \
           test_case_template_test \
           custom_exception_test fixed_mapping_test algorithms_test token_iterator_test \
           boost_check_equal_str test_tree_management_test
SRC_CPP = ${BOOST_FS_SRC}/class_properties_test.cpp
basic_cstring_test_SRC_CPP = ${BOOST_FS_SRC}/basic_cstring_test.cpp
output_test_stream_test_SRC_CPP = ${BOOST_FS_SRC}/output_test_stream_test.cpp
parameterized_test_test_SRC_CPP = ${BOOST_FS_SRC}/parameterized_test_test.cpp
test_fp_comparisons_SRC_CPP = ${BOOST_FS_SRC}/test_fp_comparisons.cpp
test_case_template_test_SRC_CPP = ${BOOST_FS_SRC}/test_case_template_test.cpp
custom_exception_test_SRC_CPP = ${BOOST_FS_SRC}/custom_exception_test.cpp
fixed_mapping_test_SRC_CPP = ${BOOST_FS_SRC}/fixed_mapping_test.cpp
algorithms_test_SRC_CPP = ${BOOST_FS_SRC}/algorithms_test.cpp
token_iterator_test_SRC_CPP = ${BOOST_FS_SRC}/token_iterator_test.cpp
boost_check_equal_str_SRC_CPP = ${BOOST_FS_SRC}/boost_check_equal_str.cpp
test_tree_management_test_SRC_CPP = ${BOOST_FS_SRC}/test_tree_management_test.cpp

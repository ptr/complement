// g++ -g -Wall -o test_parser_lowlevel4 test_parser_lowlevel4.cpp `pkg-config --cflags --libs avisynth-3.0` -lboost_unit_test_framework-gcc-mt-p-1_33_1 -lboost_thread-gcc-mt-p-1_33_1

// g++ -g -Wall -o test_parser_lowlevel4 test_parser_lowlevel4.cpp -I$HOME/local/include/boost-1_33_1 -pthread -L$HOME/local/lib -lboost_unit_test_framework-gcc-mt-p-1_33_1 -lboost_thread-gcc-mt-p-1_33_1



// STL includes
#include <iostream>
#include <sstream>
#include <string>

// Boost includes
#include <boost/test/unit_test.hpp>

struct parse_test
{
private:

  std::string msg_;

public: // constructor

  parse_test (std::string const& msg)
    : msg_(msg) { }

public: // tests

  void test_integer ();

private:

  std::string parse_script (std::string const& src);
};

void parse_test::test_integer ()
{
  BOOST_CHECK_EQUAL (parse_script ("2"), "2");
  BOOST_CHECK_EQUAL (parse_script ("3"), "2");
}

std::string parse_test::parse_script (std::string const& src)
{
  return src;
}

struct parse_test_suite : public boost::unit_test::test_suite
{
  parse_test_suite (std::string const& msg)
    : test_suite ("low level test")
  {
    boost::shared_ptr<parse_test> instance (new parse_test (msg));

    boost::unit_test::test_case *integer_test_case =
      BOOST_CLASS_TEST_CASE (&parse_test::test_integer, instance);
    add (integer_test_case, 1);
  }
};

boost::unit_test::test_suite *
init_unit_test_suite (int argc, char *argv[])
{
  boost::unit_test::test_suite *tests = BOOST_TEST_SUITE ("unit test");
  std::string msg = "2";

  tests->add (new parse_test_suite (msg));

  return tests;
}

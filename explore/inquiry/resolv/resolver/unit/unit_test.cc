// -*- C++ -*- Time-stamp: <05/11/16 16:01:46 ptr>

#include <boost/test/unit_test.hpp>

using namespace boost::unit_test_framework;

void test_basic();
void test_name_by_ip();
void test_local();
void test_mroute();
void test_hosts();

#ifdef WIN32
test_suite *__cdecl init_unit_test_suite( int argc, char * * const argv )
#else
test_suite *init_unit_test_suite( int argc, char * * const argv )
#endif /* !WIN32 */
{
  test_suite *ts = BOOST_TEST_SUITE( "libresolver test" );

  ts->add( BOOST_TEST_CASE( &test_basic ) );
  ts->add( BOOST_TEST_CASE( &test_name_by_ip ) );
  ts->add( BOOST_TEST_CASE( &test_local ) );
  ts->add( BOOST_TEST_CASE( &test_mroute ) );
  ts->add( BOOST_TEST_CASE( &test_hosts ) );

  return ts;
}

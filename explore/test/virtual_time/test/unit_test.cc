// -*- C++ -*- Time-stamp: <07/03/07 16:38:24 ptr>

#include <boost/test/unit_test.hpp>

#include <boost/lexical_cast.hpp>

#include <string>
#include <iostream>

#include <vtime.h>

using namespace boost::unit_test_framework;
using namespace vt;
using namespace std;

struct vtime_operations
{
  void vt_compare();
  void vt_add();
  void vt_diff();
  void vt_max();
};

void vtime_operations::vt_compare()
{
  vtime_type vt1;
  vtime_type vt2;

  vt1.push_back( make_pair( 1, 1 ) );
  vt1.push_back( make_pair( 2, 1 ) );

  vt2.push_back( make_pair( 1, 1 ) );
  vt2.push_back( make_pair( 2, 1 ) );

  BOOST_CHECK( vt1 == vt2 );
  BOOST_CHECK( vt1 <= vt2 );
  BOOST_CHECK( vt2 <= vt1 );

  vt2.push_back( make_pair( 3, 1 ) );

  BOOST_CHECK( !(vt1 == vt2) );
  BOOST_CHECK( vt1 <= vt2 );
  BOOST_CHECK( !(vt2 <= vt1) );

  vt1.clear();
  vt2.clear();

  vt1.push_back( make_pair( 1, 1 ) );

  vt2.push_back( make_pair( 1, 1 ) );
  vt2.push_back( make_pair( 3, 1 ) );

  BOOST_CHECK( !(vt1 == vt2) );
  BOOST_CHECK( vt1 <= vt2 );
  BOOST_CHECK( !(vt2 <= vt1) );
  
  vt1.push_back( make_pair( 2, 1 ) );

  BOOST_CHECK( !(vt1 <= vt2) );
  BOOST_CHECK( !(vt2 <= vt1) );
}

void vtime_operations::vt_add()
{
  vtime_type vt1;
  vtime_type vt2;
  vtime_type vt3;
  vtime_type vt4;

  vt1.push_back( make_pair( 1, 1 ) );
  vt1.push_back( make_pair( 2, 1 ) );

  vt3 = vt1 + vt2;

  BOOST_CHECK( vt1 == vt3 );

  vt2.push_back( make_pair( 2, 1 ) );

  vt3 = vt1 + vt2;

  vt4.push_back( make_pair( 1, 1 ) );
  vt4.push_back( make_pair( 2, 2 ) );

  BOOST_CHECK( vt3 == vt4 );

  vt4.clear();

  vt2.push_back( make_pair( 3, 1 ) );

  vt3 = vt1 + vt2;

  vt4.push_back( make_pair( 1, 1 ) );
  vt4.push_back( make_pair( 2, 2 ) );
  vt4.push_back( make_pair( 3, 1 ) );

  BOOST_CHECK( vt3 == vt4 );
}

void vtime_operations::vt_diff()
{
  vtime_type vt1;
  vtime_type vt2;
  vtime_type vt3;
  vtime_type vt4;

  vt1.push_back( make_pair( 1, 1 ) );
  vt1.push_back( make_pair( 2, 1 ) );

  vt3 = vt1 - vt2;

  BOOST_CHECK( vt1 == vt3 );

  vt2.push_back( make_pair( 1, 1 ) );

  vt3 = vt1 - vt2;

  vt4.push_back( make_pair( 2, 1 ) );

  BOOST_CHECK( vt3 == vt4 );

  vt2.push_back( make_pair( 2, 1 ) );

  vt4.clear();

  vt3 = vt1 - vt2;

  BOOST_CHECK( vt3 == vt4 );

  vt2.clear();

  vt2.push_back( make_pair( 3, 1 ) );
  
  try {
    vt3 = vt1 - vt2;
    BOOST_CHECK( false );
  }
  catch ( const std::range_error& err ) {
    BOOST_CHECK( true );
  }

  vt2.clear();

  vt2.push_back( make_pair( 2, 2 ) );

  try {
    vt3 = vt1 - vt2;
    BOOST_CHECK( false );
  }
  catch ( const std::range_error& err ) {
    BOOST_CHECK( true );
  }  
}

void vtime_operations::vt_max()
{
  vtime_type vt1;
  vtime_type vt2;
  vtime_type vt3;
  vtime_type vt4;

  vt1.push_back( make_pair( 1, 1 ) );
  vt1.push_back( make_pair( 2, 1 ) );

  vt3 = vt::max( vt1, vt2 );

  BOOST_CHECK( vt3 == vt1 );

  vt2.push_back( make_pair( 1, 1 ) );
  
  vt3 = vt::max( vt1, vt2 );

  BOOST_CHECK( vt3 == vt1 );

  vt2.push_back( make_pair( 2, 1 ) );

  vt3 = vt::max( vt1, vt2 );

  BOOST_CHECK( vt3 == vt1 );

  vt2.push_back( make_pair( 3, 1 ) );

  vt3 = vt::max( vt1, vt2 );

  vt4.push_back( make_pair( 1, 1 ) );
  vt4.push_back( make_pair( 2, 1 ) );
  vt4.push_back( make_pair( 3, 1 ) );
 
  BOOST_CHECK( vt3 == vt4 );

  vt2.clear();

  vt2.push_back( make_pair( 1, 1 ) );
  vt2.push_back( make_pair( 2, 2 ) );

  vt4.clear();

  vt3 = vt::max( vt1, vt2 );

  vt4.push_back( make_pair( 1, 1 ) );
  vt4.push_back( make_pair( 2, 2 ) );

  BOOST_CHECK( vt3 == vt4 );

  vt2.push_back( make_pair( 3, 4 ) );

  vt3 = vt::max( vt1, vt2 );

  vt4.push_back( make_pair( 3, 4 ) );

  BOOST_CHECK( vt3 == vt4 );
}

struct vtime_test_suite :
    public boost::unit_test_framework::test_suite
{
    vtime_test_suite();
};

vtime_test_suite::vtime_test_suite() :
    test_suite( "vtime test suite" )
{
  boost::shared_ptr<vtime_operations> vt_op_instance( new vtime_operations() );

  test_case *vt_compare_tc = BOOST_CLASS_TEST_CASE( &vtime_operations::vt_compare, vt_op_instance );
  test_case *vt_add_tc = BOOST_CLASS_TEST_CASE( &vtime_operations::vt_add, vt_op_instance );
  test_case *vt_diff_tc = BOOST_CLASS_TEST_CASE( &vtime_operations::vt_diff, vt_op_instance );
  test_case *vt_max_tc = BOOST_CLASS_TEST_CASE( &vtime_operations::vt_max, vt_op_instance );

  // long_msg_tc->depends_on( init_tc );

  add( vt_compare_tc );
  add( vt_add_tc );
  add( vt_diff_tc );
  add( vt_max_tc );
  // add( service_tc );
}

test_suite *init_unit_test_suite( int argc, char **argv )
{
  test_suite *ts = BOOST_TEST_SUITE( "vtime test" );
  ts->add( new vtime_test_suite() );

  return ts;
}

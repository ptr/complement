// -*- C++ -*- Time-stamp: <07/02/06 10:08:47 ptr>

/*
 * Copyright (c) 2002, 2003, 2004, 2006
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#include <boost/test/unit_test.hpp>
#include <config/feature.h>

#include "mt_test_suite.h"

using namespace boost::unit_test_framework;

void timespec_diff();
void signal_1_test();
void signal_2_test();
void signal_3_test();
void flock_test();
void lfs_test();


#ifdef WIN32
test_suite *__cdecl init_unit_test_suite( int argc, char * * const argv )
#else
test_suite *init_unit_test_suite( int argc, char * * const argv )
#endif /* !WIN32 */
{
  test_suite *ts = BOOST_TEST_SUITE( "libxmt test" );

  ts->add( BOOST_TEST_CASE( &timespec_diff ) );
  ts->add( BOOST_TEST_CASE( &signal_1_test ) );
  // You can't throw exception from signal handler
  // (stack saved/restored, that confuse stack unwind);
  // by this reason next test is commented:
  // ts->add( BOOST_TEST_CASE( &signal_2_test ) );
  ts->add( BOOST_TEST_CASE( &signal_3_test ) );

  // flock requre revision, commented now.
  // ts->add( BOOST_TEST_CASE( &flock_test ) );
  // ts->add( BOOST_TEST_CASE( &lfs_test ) );

  ts->add( new mt_test_suite() );

  return ts;
}

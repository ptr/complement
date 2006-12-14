// -*- C++ -*- Time-stamp: <06/12/14 10:01:14 ptr>

/*
 * Copyright (c) 2002, 2003, 2004, 2006
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#include <boost/test/unit_test.hpp>
#include <config/feature.h>

using namespace boost::unit_test_framework;

void timespec_diff();
void join_test();
void mutex_test();
#ifdef __FIT_PTHREAD_SPINLOCK
void spinlock_test();
#endif // __FIT_PTHREAD_SPINLOCK
void recursive_mutex_test();
void signal_1_test();
void signal_2_test();
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
  ts->add( BOOST_TEST_CASE( &join_test ) );
  ts->add( BOOST_TEST_CASE( &mutex_test ) );
#ifdef __FIT_PTHREAD_SPINLOCK
  ts->add( BOOST_TEST_CASE( &spinlock_test ) );
#endif
  ts->add( BOOST_TEST_CASE( &recursive_mutex_test ) );
  ts->add( BOOST_TEST_CASE( &signal_1_test ) );
  // You can't throw exception from signal handler
  // (stack saved/restored, that confuse stack unwind);
  // by this reason next test is commented:
  // ts->add( BOOST_TEST_CASE( &signal_2_test ) );

  // flock requre revision, commented now.
  // ts->add( BOOST_TEST_CASE( &flock_test ) );
  // ts->add( BOOST_TEST_CASE( &lfs_test ) );

  return ts;
}

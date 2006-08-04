// -*- C++ -*- Time-stamp: <04/08/31 23:07:30 ptr>

/*
 *
 * Copyright (c) 2002, 2003, 2004
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 2.0
 *
 * This material is provided "as is", with absolutely no warranty expressed
 * or implied. Any use is at your own risk.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.
 */

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "@(#)$Id$"
#  else
#ident "@(#)$Id$"
#  endif
#endif

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
  ts->add( BOOST_TEST_CASE( &flock_test ) );
  ts->add( BOOST_TEST_CASE( &lfs_test ) );

  return ts;
}

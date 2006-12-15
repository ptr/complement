// -*- C++ -*- Time-stamp: <06/12/15 10:38:16 ptr>

/*
 * Copyright (c) 2002, 2003, 2006
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#include <boost/test/unit_test.hpp>

using namespace boost::unit_test_framework;

#include <mt/xmt.h>

using namespace std;
using namespace xmt;

static int v = 0;

#ifdef __FIT_PTHREAD_SPINLOCK
static Spinlock sl1;

xmt::Thread::ret_code thr1_sl( void * )
{
  sl1.lock();
  BOOST_CHECK( v == 0 );

  delay( xmt::timespec( 1, 0 ) );

  BOOST_CHECK( v == 0 );
  v = 1;

  sl1.unlock();

  xmt::Thread::ret_code rt;
  rt.iword = 0;

  return rt;
}

xmt::Thread::ret_code thr2_sl( void * )
{
  sl1.lock();
  BOOST_CHECK( v == 1);
  v = 2;
  sl1.unlock();

  xmt::Thread::ret_code rt;
  rt.iword = 0;

  return rt;
}

xmt::Thread::ret_code thr3_sl( void * )
{
  sl1.lock();

  BOOST_CHECK( v == 0 );

  delay( xmt::timespec( 1, 0 ) );

  v = 1;

  sl1.unlock();

  xmt::Thread::ret_code rt;
  rt.iword = 0;

  return rt;
}

xmt::Thread::ret_code thr4_sl( void * )
{
  if ( sl1.trylock() == 0 ) {
    BOOST_CHECK( v == 1 );
    sl1.unlock();
    BOOST_ERROR( "m1.trylock() return zero!" );
  }

  xmt::Thread::ret_code rt;
  rt.iword = 0;

  return rt;
}

void spinlock_test()
{
  v = 0;
  Thread t1( thr1_sl );
  Thread t2( thr2_sl );

  t1.join();
  t2.join();

  BOOST_CHECK( v == 2 );

  v = 0;
  Thread t3( thr3_sl );
  Thread t4( thr4_sl );

  t3.join();
  t4.join();
}
#endif // __FIT_PTHREAD_SPINLOCK

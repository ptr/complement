// -*- C++ -*- Time-stamp: <06/12/15 10:37:09 ptr>

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

static Mutex m1;

static int v = 0;
static volatile int msync = 0;

xmt::Thread::ret_code thr1( void * )
{
  m1.lock();
  msync = 1;
  BOOST_CHECK( v == 0 );

  delay( xmt::timespec( 1, 0 ) );

  BOOST_CHECK( v == 0 );
  v = 1;

  m1.unlock();

  xmt::Thread::ret_code rt;
  rt.iword = 0;

  return rt;
}

xmt::Thread::ret_code thr2( void * )
{
  int j = 0;
  while ( !msync ) {
    ++j;
  }
  m1.lock();
  BOOST_CHECK( v == 1);
  v = 2;
  m1.unlock();

  xmt::Thread::ret_code rt;
  rt.iword = 0;

  return rt;
}

xmt::Thread::ret_code thr3( void * )
{
  m1.lock();
  msync = 1;

  BOOST_CHECK( v == 0 );

  delay( xmt::timespec( 1, 0 ) );

  v = 1;

  m1.unlock();
  xmt::Thread::ret_code rt;
  rt.iword = 0;

  return rt;
}

xmt::Thread::ret_code thr4( void * )
{
#ifndef _WIN32 // no trylock
  int j = 0;
  while ( !msync ) {
    ++j;
  }
  if ( m1.trylock() == 0 ) {
    BOOST_CHECK( v == 1 );
    m1.unlock();
    BOOST_ERROR( "m1.trylock() return zero!" );
  }
#endif // _WIN32
  
  xmt::Thread::ret_code rt;
  rt.iword = 0;

  return rt;
}


void mutex_test()
{
  v = 0;
  Thread t1( thr1 );
  Thread t2( thr2 );

  t1.join();
  t2.join();

  msync = 0;

  BOOST_CHECK( v == 2 );

#ifndef _WIN32
  v = 0;
  Thread t3( thr3 );
  Thread t4( thr4 );

  t3.join();
  t4.join();
#endif // _WIN32
}

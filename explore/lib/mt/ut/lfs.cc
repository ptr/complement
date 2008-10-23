// -*- C++ -*- Time-stamp: <07/09/05 00:01:28 ptr>

/*
 * Copyright (c) 2004, 2006, 2007
 * Petr Ovtchenkov
 *
 * Copyright (c) 2004
 * Kaspersky Lab
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#include <exam/suite.h>

#include "mt/lfstream.h"
#include <mt/xmt.h>

#include <sys/file.h>
#include <unistd.h>
#include <errno.h>

#include <iostream>

using namespace xmt;
using namespace std;

static const char *fname = "myfile";
static mutex m;
static mutex b;
static int cnt = 0;
static condition cnd;

static Thread::ret_t thread_func( void * )
{
  try {
    for ( int count = 0; count < 10; ++count ) {
      olfstream s( fname, ios_base::out | ios_base::app );

      b.lock();
      BOOST_REQUIRE( s.good() );
      b.unlock();

      s.lock_ex();
      
      b.lock();
      BOOST_REQUIRE( s.good() );
      b.unlock();

      s << "test string 1\n";
      s << "test string 2\n";
      s << "test string 3\n";

      s.flush();
      
      m.lock();
      ++cnt;
      m.unlock();

      cnd.set( true, true );

      delay( xmt::timespec(1,0) );

      s.unlock();

      b.lock();
      BOOST_REQUIRE( s.good() );
      b.unlock();

      s.close();
    }
  }
  catch ( ... ) {
    BOOST_REQUIRE( false );
    return reinterpret_cast<Thread::ret_t>(-1);
  }

  return 0;
}

static Thread::ret_t thread_func_r( void * )
{
  try {
    string buf;
    int ln = 0;

    cnd.try_wait();

    for ( int count = 0; count < 10; ++count ) {
      ilfstream s( fname );
      ln = 0;
    
      b.lock();
      BOOST_REQUIRE( s.good() );
      b.unlock();

      s.lock_sh();

      b.lock();
      BOOST_REQUIRE( s.good() );
      b.unlock();

      m.lock();
      int ready_cnt = cnt;
      m.unlock();

      while ( ready_cnt-- > 0 ) {
        getline( s, buf );
        ++ln;
        b.lock();
        BOOST_CHECK( buf == "test string 1" );
        b.unlock();
        getline( s, buf );
        ++ln;
        b.lock();
        BOOST_CHECK( buf == "test string 2" );
        b.unlock();
        getline( s, buf );
        ++ln;
        b.lock();
        BOOST_CHECK( buf == "test string 3" );
        b.unlock();
      }

      delay( xmt::timespec(1,0) );

      s.unlock();
      b.lock();
      BOOST_REQUIRE( s.good() );
      b.unlock();
      s.close();
    }
  }
  catch ( ... ) {
    b.lock();
    BOOST_REQUIRE( false );
    b.unlock();
    return reinterpret_cast<Thread::ret_t>(-1);
  }

  return 0;
}

static Thread::ret_t thread_func_manip( void * )
{
  for ( int count = 0; count < 10; ++count ) {
    olfstream s( fname, ios_base::out | ios_base::app );

    b.lock();
    BOOST_REQUIRE( s.good() );
    b.unlock();

    s << elck
      << "test string 1\n"
      << "test string 2\n"
      << "test string 3\n";

    s.flush();

    m.lock();
    ++cnt;
    m.unlock();

    cnd.set( true, true );

    delay( xmt::timespec(1,0) );

    s << ulck;

    b.lock();
    BOOST_CHECK( s.good() );
    b.unlock();

    s.close();
  }

  return 0;
}

static Thread::ret_t thread_func_manip_r( void * )
{
  string buf;

  cnd.try_wait();

  for ( int count = 0; count < 10; ++count ) {
    ilfstream s( fname );
    
    b.lock();
    BOOST_REQUIRE( s.good() );
    b.unlock();

    s >> slck;

    m.lock();
    int ready_cnt = cnt;
    m.unlock();

    while ( ready_cnt-- > 0 ) {
      getline( s, buf );
      b.lock();
      BOOST_CHECK( buf == "test string 1" );
      b.unlock();
      getline( s, buf );
      b.lock();
      BOOST_CHECK( buf == "test string 2" );
      b.unlock();
      getline( s, buf );
      b.lock();
      BOOST_CHECK( buf == "test string 3" );
      b.unlock();
    }

    delay( xmt::timespec(1,0) );

    s >> ulck;
    b.lock();
    BOOST_REQUIRE( s.good() );
    b.unlock();
    s.close();
  }

  return 0;
}

int EXAM_IMPL(lfs_test)
{
  unlink( fname );
  cnd.set( false );

  Thread t1( thread_func );
  Thread t2( thread_func );
  Thread t3( thread_func_r );
  Thread t4( thread_func_r );

  t1.join();
  t2.join();
  t3.join();
  t4.join();

  unlink( fname );
#if 0
  cnt = 0;
  cnd.set( false );

  Thread tm1( thread_func_manip );
  Thread tm2( thread_func_manip );
  Thread tm3( thread_func_manip_r );
  Thread tm4( thread_func_manip_r );

  tm1.join();
  tm2.join();
  tm3.join();
  tm4.join();

  unlink( fname );
#endif

  return EXAM_RESULT;
}

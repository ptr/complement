// -*- C++ -*- Time-stamp: <06/08/04 11:25:15 ptr>

/*
 *
 * Copyright (c) 2004, 2006
 * Petr Ovtchenkov
 *
 * Copyright (c) 2004
 * Kaspersky Lab
 *
 * Licensed under the Academic Free License Version 2.1
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

#include <boost/test/unit_test.hpp>
#include "mt/lfstream.h"
#include <mt/xmt.h>

#include <sys/file.h>
#include <unistd.h>
#include <errno.h>

#include <iostream>

using namespace boost::unit_test_framework;

using namespace xmt;
using namespace std;

static const char *fname = "myfile";
static Mutex m;
static Mutex b;
static int cnt = 0;
static Condition cnd;

static Thread::ret_code thread_func( void * )
{
  Thread::ret_code rt;
  rt.iword = 0;

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

      timespec t;
      t.tv_sec = 1;
      t.tv_nsec = 0;

      Thread::delay( &t );

      s.unlock();

      b.lock();
      BOOST_REQUIRE( s.good() );
      b.unlock();

      s.close();
    }
  }
  catch ( ... ) {
    BOOST_REQUIRE( false );
    rt.iword = -1;
  }

  return rt;
}

static Thread::ret_code thread_func_r( void * )
{
  Thread::ret_code rt;
  rt.iword = 0;

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

      timespec t;
      t.tv_sec = 1;
      t.tv_nsec = 0;

      Thread::delay( &t );

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
    rt.iword = -1;
  }

  return rt;
}

static Thread::ret_code thread_func_manip( void * )
{
  Thread::ret_code rt;
  rt.iword = 0;

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

    timespec t;
    t.tv_sec = 1;
    t.tv_nsec = 0;

    Thread::delay( &t );

    s << ulck;

    b.lock();
    BOOST_CHECK( s.good() );
    b.unlock();

    s.close();
  }

  return rt;
}

static Thread::ret_code thread_func_manip_r( void * )
{
  Thread::ret_code rt;
  rt.iword = 0;

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

    timespec t;
    t.tv_sec = 1;
    t.tv_nsec = 0;

    Thread::delay( &t );

    s >> ulck;
    b.lock();
    BOOST_REQUIRE( s.good() );
    b.unlock();
    s.close();
  }

  return rt;
}

void lfs_test()
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
}
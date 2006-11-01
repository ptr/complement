// -*- C++ -*- Time-stamp: <06/08/04 11:16:33 ptr>

/*
 * Copyright (c) 2003, 2006
 * Petr Ovtchenkov
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

using namespace boost::unit_test_framework;

#include <mt/xmt.h>

#include <iostream>

using namespace std;
using namespace xmt;

static Thread::ret_code thread_one( void * );
static Thread::ret_code thread_two( void * );

static Thread *th_one = 0;
static Thread *th_two = 0;

static int v = 0;

extern "C" {
  static void handler( int );

  void handler( int sig )
  {
    BOOST_CHECK( v == 1 );
    v = 4;
    // cerr << "thread_one: Handler" << endl;

    throw sig;

    v = 3; // not reached
  }
}

Thread::ret_code thread_one( void * )
{
  Thread::ret_code rt;
  rt.iword = 0;

  try {
    Thread::unblock_signal( SIGINT ); // we wait this signal
    // I set own handler, that throw signal:
    Thread::signal_handler( SIGINT, handler );

    BOOST_CHECK( v == 1 );

    // wait while set thread's pointer (from thread_two)
    timespec t;

    t.tv_sec = 1;
    t.tv_nsec = 0;

    while ( th_one == 0 ) {
      Thread::sleep( &t );
    }

//  pm.lock();
//  cerr << "thread_one: send SIGINT" << endl;
//  pm.unlock();
    th_one->kill( SIGINT ); // send signal SIGINT to self
//  pm.lock();
//  cerr << "thread_one: after send SIGINT" << endl;
//  pm.unlock();

    v = 2; // not reached (exit before)
  }
  catch ( int s ) {
    BOOST_CHECK( v == 4 );
    // pm.lock();
    // cerr << "thread_one: catch signal after send SIGINT" << endl;
    // pm.unlock();
    v = 5;
  }

  return rt;
}

Thread::ret_code thread_two( void * )
{
  try {
    // pm.lock();
    // cerr << "thread_two: create thread one" << endl;
    // pm.unlock();

    v = 1;

    Thread t( thread_one ); // start thread_one
    th_one = &t; // store address for call later

    // pm.lock();
    // cerr << "thread_two: wait termination of thread one" << endl;
    // pm.unlock();
    t.join();
    // pm.lock();
    // cerr << "thread_two: EOL of thread one" << endl;
    // pm.unlock();

    BOOST_CHECK( v == 5 );
  }
  catch ( int s ) {
    cerr << "***********" << endl;
    // BOOST_CHECK( v == 4 );
    // v = 5;
  }

  Thread::ret_code rt;
  rt.iword = 0;

  return rt;
}

void signal_2_test()
{
  Thread t( thread_two );

  t.join();

  BOOST_CHECK( v == 5 );
}

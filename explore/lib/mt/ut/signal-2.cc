// -*- C++ -*- Time-stamp: <07/09/05 00:04:38 ptr>

/*
 * Copyright (c) 2003, 2006, 2007
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#include <exam/suite.h>

#include <mt/xmt.h>

#include <iostream>

using namespace std;
using namespace xmt;

static Thread::ret_t thread_one( void * );
static Thread::ret_t thread_two( void * );

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

Thread::ret_t thread_one( void * )
{
  try {
    xmt::unblock_signal( SIGINT ); // we wait this signal
    // I set own handler, that throw signal:
    xmt::signal_handler( SIGINT, handler );

    BOOST_CHECK( v == 1 );

    // wait while set thread's pointer (from thread_two)
    xmt::timespec t(1,0);

    while ( th_one == 0 ) {
      xmt::sleep( t );
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

  return 0;
}

Thread::ret_t thread_two( void * )
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

  return 0;
}

int EXAM_IMPL(signal_2_test)
{
  Thread t( thread_two );

  t.join();

  EXAM_CHECK( v == 5 );

  return EXAM_RESULT;
}

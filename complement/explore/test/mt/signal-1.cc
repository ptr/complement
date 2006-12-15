// -*- C++ -*- Time-stamp: <06/12/15 10:40:53 ptr>

/*
 * Copyright (c) 2003, 2006
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#include <boost/test/unit_test.hpp>

using namespace boost::unit_test_framework;

#include <mt/xmt.h>

using namespace xmt;

static Thread::ret_code thread_one( void * );
static Thread::ret_code thread_two( void * );

static Thread *th_one = 0;
static Thread *th_two = 0;

static int v = 0;

extern "C" {
  static void handler( int );

  void handler( int )
  {
    BOOST_CHECK( v == 1 );
    v = 4;
    /*
     Note: you have very restricted list of system calls that you can use here
     (in the handler of signal) safely. In particular, you can't call pthread_*
     functions. Reason: async-signal-safe calls, Unix 98, POSIX 1002.1
     */
    // cerr << "thread_one: Handler" << endl;
    // Thread::signal_exit( SIGTERM );
    // send signal to caller thread to exit:
    // th_one->kill( SIGTERM );

    // v = 3; // not reached
  }
}

Thread::ret_code thread_one( void * )
{
  xmt::unblock_signal( SIGINT ); // we wait this signal
  // Default handler make exit() call:
  //   Thread::signal_handler( SIGINT, SIG_DFL );
  // That's why I set own handler:
  xmt::signal_handler( SIGINT, handler );

  BOOST_CHECK( v == 1 );

//  pm.lock();
//  cerr << "thread_one: unblock signal SIGINT" << endl;
//  cerr << "thread_one: set own handler for signal SIGINT" << endl;
//  pm.unlock();


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

  // v = 2; // not reached (exit before)

  Thread::ret_code rt;
  rt.iword = 0;

  return rt;
}

Thread::ret_code thread_two( void * )
{
  // pm.lock();
  // cerr << "thread_two: create thread one" << endl;
  // pm.unlock();

  v = 1;

  Thread t( thread_one ); // start thread_one
  th_one = &t; // store address to be called from thread_one

  // pm.lock();
  // cerr << "thread_two: wait termination of thread one" << endl;
  // pm.unlock();
  t.join();
  // pm.lock();
  // cerr << "thread_two: EOL of thread one" << endl;
  // pm.unlock();

  BOOST_CHECK( v == 4 );

  Thread::ret_code rt;
  rt.iword = 0;

  return rt;
}

void signal_1_test()
{
  Thread t( thread_two );

  t.join();

  BOOST_CHECK( v == 4 );
}

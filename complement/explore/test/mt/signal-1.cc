// -*- C++ -*- Time-stamp: <07/07/11 21:21:38 ptr>

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


/* 
 * thread 2:  v = 1; create thread 1 ----------------------------------- join; v == 4?
 *                      \                                          /  
 * thread 1:             set handler; v == 1? - kill ----------- exit 
 *                                                 \                
 * handler (within thread 1):                         v == 1?; v = 4
 */

static Thread::ret_code thread_one( void * );
static Thread::ret_code thread_two( void * );

static Thread *th_one = 0;
static Thread *th_two = 0;

static int v = 0;

static condition cnd;

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

  cnd.try_wait();

  th_one->kill( SIGINT ); // send signal SIGINT to self

  Thread::ret_code rt;
  rt.iword = 0;

  return rt;
}

Thread::ret_code thread_two( void * )
{
  cnd.set( false );

  v = 1;

  Thread t( thread_one ); // start thread_one
  th_one = &t;            // store address to be called from thread_one

  cnd.set( true );

  t.join();

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

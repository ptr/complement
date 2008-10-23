// -*- C++ -*- Time-stamp: <07/09/05 00:05:31 ptr>

/*
 * Copyright (c) 2003, 2006, 2007
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#include <exam/suite.h>

#include <mt/xmt.h>

using namespace xmt;

/*
 * This is the same as signal-1, but instead of unblock signal, I block one
 * so I don't take this signal.
 */

/* 
 * handler (within thread 2):                           v == 1?; v = 4
 *                                                    /
 * thread 2:  v = 1; create thread 1 ----------------------------------- join; v == 4?
 *                      \                          /            /
 * thread 1:             set handler; v == 1? - kill ------ exit 
 *
 */

static Thread::ret_t thread_one( void * );
static Thread::ret_t thread_two( void * );

static Thread *th_one = 0;
static Thread *th_two = 0;

static int v = 0;

static condition cnd;

extern "C" {
  static void handler( int );

  void handler( int )
  {
    EXAM_CHECK_ASYNC( v == 1 );
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

Thread::ret_t thread_one( void * )
{
  EXAM_CHECK_ASYNC( v == 1 );

  cnd.try_wait();

  th_two->kill( SIGINT ); // send signal SIGINT to self

  return 0;
}

Thread::ret_t thread_two( void * )
{
  xmt::signal_handler( SIGINT, handler );
  xmt::block_signal( SIGINT ); // block signal

  v = 1;

  Thread t( thread_one ); // start thread_one

  t.join();

  EXAM_CHECK_ASYNC( v == 1 ); // signal was blocked!

  xmt::unblock_signal( SIGINT ); // unblock signal

  EXAM_CHECK_ASYNC( v == 4 );

  return 0;
}

int EXAM_IMPL(signal_3_test)
{
  cnd.set( false );

  Thread t( thread_two );

  th_two = &t;            // store address to be called from thread_one

  cnd.set( true );

  t.join();

  EXAM_CHECK( v == 4 );

  return EXAM_RESULT;
}

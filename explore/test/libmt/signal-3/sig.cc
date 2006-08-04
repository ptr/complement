// -*- C++ -*- Time-stamp: <02/09/25 12:17:30 ptr>

/*
 * Copyright (c) 2002
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 1.0
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


/*
 * 1. In main thread we create thread_two and wait it termination.
 * 
 * 2. Then from thread thread_two we create thread thread_one and wait for
 *    ones termination.
 *
 * 3. In thread thread_one we send signal SIGINT to self
 *    (i.e. to thread thread_one)
 *
 * The salt is: with default SIGINT handler
 *
 *     Thread::signal_handler( SIGINT, SIG_DFL );
 *
 * that provide exit() system call.
 * 
 */

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "@(#)$Id$"
#  else
#ident "@(#)$Id$"
#  endif
#endif

#include <mt/xmt.h>
#include <iostream>

using namespace __impl;
using namespace std;

Thread *th_one = 0;
Thread *th_two = 0;

Mutex pm;

extern "C" {
  void handler( int )
  {
    pm.lock();
    cerr << "thread_one: Handler" << endl;
    pm.unlock();
    Thread::signal_exit( SIGTERM );
  }
}

int thread_one( void * )
{
  Thread::unblock_signal( SIGINT ); // we wait this signal
  // Default handler make exit() call:
  // Thread::signal_handler( SIGINT, SIG_DFL );
  // That's why I set own handler:
  Thread::signal_handler( SIGINT, handler );

  pm.lock();
  cerr << "thread_one: unblock signal SIGINT" << endl;
  cerr << "thread_one: set own handler for signal SIGINT" << endl;
  pm.unlock();


  // wait while set thread's pointer (from thread_two)
  timespec t;

  t.tv_sec = 1;
  t.tv_nsec = 0;

  while ( th_one == 0 ) {
    Thread::sleep( &t );
  }

  pm.lock();
  cerr << "thread_one: send SIGINT" << endl;
  pm.unlock();
  th_one->kill( SIGINT ); // send signal SIGINT to self
  pm.lock();
  cerr << "thread_one: after send SIGINT" << endl;
  pm.unlock();

  return 0;
}

int thread_two( void * )
{
  // Thread::block_signal( SIGINT ); // see comment below

  // If we block SIGINT here, it will be passed to parent thread,
  // in this case main. So we unblock SIGINT, but ignore.
//  Thread::unblock_signal( SIGINT );
  // Thread::signal_handler( SIGINT, SIG_IGN );
//  Thread::signal_handler( SIGINT, handler );

//pm.lock();
//cerr << "block signal SIGINT on thread_two" << endl;
//pm.unlock();

  pm.lock();
  cerr << "thread_two: create thread one" << endl;
  pm.unlock();

  Thread t( thread_one ); // start thread_one
  th_one = &t; // store address for call later

  // pm.lock();
  // cerr << "thred_two: wait termination of thread one" << endl;
  // pm.unlock();
  t.join();
  pm.lock();
  cerr << "thred_two: EOL of thread one" << endl;
  pm.unlock();

  return 0;
}


int main( int, char * const * )
{
  // Thread::block_signal( SIGINT );
  // Thread::signal_handler( SIGINT, handler );

  pm.lock();
  cerr << "main: create thread two" << endl;
  pm.unlock();
  Thread t( thread_two );

  // pm.lock();
  // cerr << "main: wait termination of thread two" << endl;
  // pm.unlock();

  t.join();

  pm.lock();
  cerr << "main: EOL of thread two" << endl;
  pm.unlock();

  return 0;
}

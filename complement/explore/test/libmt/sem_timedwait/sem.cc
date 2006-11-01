// -*- C++ -*- Time-stamp: <03/02/25 18:01:39 ptr>

/*
 * Copyright (c) 2003
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
 * Test for recursive-safe mutexes (detect deadlock)
 *
 * 1. Start thread 1. Acquire lock on m. Sleep hread 1 sec.
 *
 * 2. Start thread 2. Acquire lock on m. Due to m locked
 *    in thread 1, waiting on m.lock.
 *
 * 3. From thread 1 call function 'recursive', where acquire lock on m,
 *    sleep 1 sec, and release m.
 *    If mutex recursive-safe, function 'recursive' will finished correctly.
 *    Otherwise, deedlock will happen on m.lock in 'recursive'.
 *
 * 4. Release m in thread 1.
 *
 * 5. Pass through m lock in thread 2. Call function 'recursive',
 *    where acquire lock on m, sleep 1 sec, and release m. See item 3 before.
 *
 * 6. Release m in thread 2.
 *
 * 7. Program finished.
 * 
 */

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "@(#)$Id$"
#  else
#ident "@(#)$Id$"
#  endif
#endif

// #define _STLP_ASSERTIONS 1

#include <mt/xmt.h>
#include <iostream>

using namespace __impl;
using namespace std;

Mutex pm;

Semaphore s(0);

void recursive()
{
  pm.lock();
  cerr << "before lock recursive" << endl;
  pm.unlock();

  s.wait();
  timespec t;

  t.tv_sec = 1;
  t.tv_nsec = 0;

  Thread::sleep( &t );
  s.post();
  
  pm.lock();
  cerr << "after lock recursive" << endl;
  pm.unlock();
}

int thread_one( void * )
{
  pm.lock();
  cerr << "before wait in thread one" << endl;
  pm.unlock();

  timespec t;

  t.tv_sec = 2;
  t.tv_nsec = 0;

  s.wait_delay( &t );

  pm.lock();
  cerr << "after wait in thread one" << endl;
  pm.unlock();

  t.tv_sec = 4;
  t.tv_nsec = 0;

  Thread::sleep( &t );

  // recursive();  

  pm.lock();
  cerr << "before post in thread one" << endl;
  pm.unlock();

  s.post();

  pm.lock();
  cerr << "after post in thread one" << endl;
  pm.unlock();

  return 0;
}

int thread_two( void * )
{
  pm.lock();
  cerr << "before wait in thread two" << endl;
  pm.unlock();

  timespec t;

  t.tv_sec = 2;
  t.tv_nsec = 0;

  s.wait_delay( &t );

  pm.lock();
  cerr << "after wait in thread two" << endl;
  pm.unlock();

  t.tv_sec = 4;
  t.tv_nsec = 0;

  Thread::sleep( &t );
  // recursive();  

  pm.lock();
  cerr << "before post in thread two" << endl;
  pm.unlock();

  s.post();

  pm.lock();
  cerr << "after post in thread two" << endl;
  pm.unlock();

  return 0;
}


int main( int, char * const * )
{
  timespec t;

  t.tv_sec = 1;
  t.tv_nsec = 0;

  Semaphore sem0( 0 );

  pm.lock();
  cerr << "main: before wait" << endl;
  pm.unlock();
  sem0.wait_delay( &t ); // test timeout work
  
  pm.lock();
  cerr << "main: after wait" << endl;
  pm.unlock();

  // ------------------

  pm.lock();
  cerr << "main: create thread one" << endl;
  pm.unlock();

  Thread t1( thread_one );

  pm.lock();
  cerr << "main: create thread two" << endl;
  pm.unlock();

  Thread t2( thread_two );

  t1.join();
  t2.join();

  pm.lock();
  cerr << "main: wait nothing (wait time 0)" << endl;
  pm.unlock();

  timespec t0;

  t0.tv_sec = 0;
  t0.tv_nsec = 0;

  sem0.wait_delay( &t0 ); // test timeout work for time 0

  pm.lock();
  cerr << "main: wait nothing (wait time 0) pass" << endl;
  pm.unlock();


  pm.lock();
  cerr << "main: wait time in pass (wait time -1 sec)" << endl;
  pm.unlock();

  t0.tv_sec = -1; // this is work only if time_t is signed (for Linux is signed)
  t0.tv_nsec = 0;

  sem0.wait_delay( &t0 ); // test timeout work for time 0

  pm.lock();
  cerr << "main: wait time in pass (wait time -1 sec) pass" << endl;
  pm.unlock();


  pm.lock();
  cerr << "main: End" << endl;
  pm.unlock();

  return 0;
}

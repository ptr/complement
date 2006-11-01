// -*- C++ -*- Time-stamp: <03/01/19 19:51:08 ptr>

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

int thread_one( void * )
{
  pm.lock();
  cerr << "before sleep in thread one" << endl;
  pm.unlock();

  timespec t;

  t.tv_sec = 5;
  t.tv_nsec = 0;

  Thread::delay( &t );

  pm.lock();
  cerr << "after sleep in thread one" << endl;
  pm.unlock();

  return 0;
}

int thread_two( void * )
{
  pm.lock();
  cerr << "before sleep in thread two" << endl;
  pm.unlock();

  timespec t;

  t.tv_sec = 0;
  t.tv_nsec = 500000000;

  int i = 0;

  pm.lock();
  cerr << "thread two " << i++ << endl;
  pm.unlock();

  Thread::delay( &t );

  pm.lock();
  cerr << "thread two " << i++ << endl;
  pm.unlock();

  Thread::delay( &t );

  pm.lock();
  cerr << "thread two " << i++ << endl;
  pm.unlock();

  Thread::delay( &t );

  pm.lock();
  cerr << "thread two " << i++ << endl;
  pm.unlock();

  Thread::delay( &t );

  pm.lock();
  cerr << "after sleep in thread two" << endl;
  pm.unlock();

  return 0;
}


int main( int, char * const * )
{
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
  cerr << "main: End" << endl;
  pm.unlock();

  return 0;
}

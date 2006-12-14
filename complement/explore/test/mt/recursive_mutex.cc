// -*- C++ -*- Time-stamp: <06/08/04 11:07:52 ptr>

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

#include <boost/test/unit_test.hpp>

using namespace boost::unit_test_framework;

#include <mt/xmt.h>
#include <iostream>

using namespace xmt;
using namespace std;

// Mutex pm;

__Mutex<true,false> m;
static int v = 0;

void recursive()
{
  // pm.lock();
  // cerr << "before lock recursive" << endl;
  // pm.unlock();

  m.lock();
  v = 2;
  timespec t;

  t.tv_sec = 1;
  t.tv_nsec = 0;

  Thread::sleep( &t );
  BOOST_CHECK( v == 2 );
  m.unlock();
  
  // pm.lock();
  // cerr << "after lock recursive" << endl;
  // pm.unlock();
}

Thread::ret_code thread_one( void * )
{
  // pm.lock();
  // cerr << "before lock in thread one" << endl;
  // pm.unlock();

  m.lock();

  BOOST_CHECK( v == 0 );

  v = 1;

  // pm.lock();
  // cerr << "after lock in thread one" << endl;
  // pm.unlock();

  timespec t;

  t.tv_sec = 1;
  t.tv_nsec = 0;

  Thread::sleep( &t );

  BOOST_CHECK( v == 1 );

  recursive();  

  // pm.lock();
  // cerr << "before unlock in thread one" << endl;
  // pm.unlock();

  BOOST_CHECK( v == 2 );

  v = 3;

  m.unlock();

  // pm.lock();
  // cerr << "after unlock in thread one" << endl;
  // pm.unlock();

  Thread::ret_code rt;

  rt.iword = 0;

  return rt;
}

Thread::ret_code thread_two( void * )
{
  // pm.lock();
  // cerr << "before lock in thread two" << endl;
  // pm.unlock();

  m.lock();

  // pm.lock();
  // cerr << "after lock in thread two" << endl;
  // pm.unlock();
  BOOST_CHECK( v == 3 );


  timespec t;

  t.tv_sec = 1;
  t.tv_nsec = 0;

  Thread::sleep( &t );
  recursive();  

  // pm.lock();
  // cerr << "before unlock in thread two" << endl;
  // pm.unlock();

  BOOST_CHECK( v == 2 );

  m.unlock();

  // pm.lock();
  // cerr << "after unlock in thread two" << endl;
  // pm.unlock();

  Thread::ret_code rt;

  rt.iword = 0;

  return rt;
}


void recursive_mutex_test()
{
  // pm.lock();
  // cerr << "main: create thread one" << endl;
  // pm.unlock();

  Thread t1( thread_one );

  // pm.lock();
  // cerr << "main: create thread two" << endl;
  // pm.unlock();

  Thread t2( thread_two );

  t1.join();
  t2.join();

  // pm.lock();
  // cerr << "main: End" << endl;
  // pm.unlock();

  // return 0;
}

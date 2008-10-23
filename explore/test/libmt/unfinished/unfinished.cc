// -*- C++ -*- Time-stamp: <03/10/15 18:48:36 ptr>

/*
 *
 * Copyright (c) 2002, 2003
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 1.2
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

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "@(#)$Id$"
#  else
#ident "@(#)$Id$"
#  endif
#endif

// #include <boost/test/unit_test.hpp>

// using namespace boost::unit_test_framework;

#include <mt/xmt.h>
#include <vector>

using namespace std;
using namespace __impl;

Semaphore s(0);
// Condition c;

int thread_func( void * )
{
  timespec t;
  t.tv_sec = 1;
  t.tv_nsec = 0;

  while ( s.wait_delay( &t ) != 0 ) {
    Thread::delay( &t );
  }

  return 0;
}

void sig_h( int )
{
  // c.set( true );
  // c.signal();
  s.post();
}

#if 0
int control_thread_func( void * )
{
  Thread::signal_handler( SIGINT, sig_h );

  // c.wait();
  s.wait();

  return 0;
}
#endif

int main()
{
  const int n = 30;
  typedef vector<Thread *> container;
  typedef container::iterator thi_t;
  container v;

  // c.set( false );

  Thread::signal_handler( SIGINT, sig_h );

  // Thread control( control_thread_func );
  

  for ( int i = 0; i < n; ++i ) {
    v.push_back( new Thread( thread_func ) );
  }

  for ( thi_t i = v.begin(); i != v.end(); ++i ) {
    (*i)->join();
  }

  for ( thi_t i = v.begin(); i != v.end(); ++i ) {
    delete *i;
  }

  // control.join();

  return 0;
}

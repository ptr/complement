// -*- C++ -*- Time-stamp: <08/08/13 13:28:22 yeti>

/*
 * Copyright (c) 2008
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 */

#include "alloc_perf_suite.h"
#include <vector>

#include <mt/thread>
#include <mt/mutex>

using namespace std;

vector<void *> p( 1000 );

int EXAM_IMPL(alloc_test::alloc)
{
  for ( int i = 0; i < 10000; ++i ) {
    if ( p[i % 1000] != 0 ) {
      free( p[i % 1000] );
    }
    p[i % 1000] = malloc( 100 );
    if ( p[i % 1000] == 0 ) {
      return 1;
    }
  }

  return 0;
}

int EXAM_IMPL(alloc_test::alloc5000)
{
  for ( int i = 0; i < 10000; ++i ) {
    if ( p[i % 1000] != 0 ) {
      free( p[i % 1000] );
    }
    p[i % 1000] = malloc( 5000 );
    if ( p[i % 1000] == 0 ) {
      return 1;
    }
  }

  return 0;
}

int EXAM_IMPL(alloc_test::alloc_mix)
{
  for ( int i = 0; i < 5000; ++i ) {
    if ( p[i % 1000] != 0 ) {
      free( p[i % 1000] );
    }
    p[i % 1000] = malloc( 5000 );
    if ( p[i % 1000] == 0 ) {
      return 1;
    }
    if ( malloc( 100 ) == 0 ) {
      return 1;
    }
  }

  return 0;
}

static std::tr2::mutex lk;

void a0()
{
  for ( int i = 0; i < 10000; ++i ) {
    std::tr2::lock_guard<std::tr2::mutex> lock( lk );

    if ( p[i % 1000] != 0 ) {
      free( p[i % 1000] );
    }
    p[i % 1000] = malloc( 100 );
  }
}

int EXAM_IMPL(alloc_test::alloc_t5)
{
  std::tr2::basic_thread<0,0> t0( a0 );
  std::tr2::basic_thread<0,0> t1( a0 );
  std::tr2::basic_thread<0,0> t2( a0 );
  std::tr2::basic_thread<0,0> t3( a0 );
  std::tr2::basic_thread<0,0> t4( a0 );

  t0.join();
  t1.join();
  t2.join();
  t3.join();
  t4.join();

  return 0;
}

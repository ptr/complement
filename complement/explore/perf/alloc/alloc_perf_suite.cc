// -*- C++ -*- Time-stamp: <08/08/12 18:28:14 yeti>

/*
 * Copyright (c) 2008
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 */

#include "alloc_perf_suite.h"

using namespace std;

int EXAM_IMPL(alloc_test::alloc)
{
  for ( int i = 0; i < 10000; ++i ) {
    if ( malloc( 100 ) == 0 ) {
      return 1;
    }
  }

  return 0;
}

int EXAM_IMPL(alloc_test::alloc5000)
{
  for ( int i = 0; i < 10000; ++i ) {
    if ( malloc( 5000 ) == 0 ) {
      return 1;
    }
  }

  return 0;
}

int EXAM_IMPL(alloc_test::alloc_mix)
{
  for ( int i = 0; i < 5000; ++i ) {
    if ( malloc( 5000 ) == 0 ) {
      return 1;
    }
    if ( malloc( 100 ) == 0 ) {
      return 1;
    }
  }

  return 0;
}

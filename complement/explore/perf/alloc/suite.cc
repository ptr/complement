// -*- C++ -*- Time-stamp: <08/08/12 16:48:21 yeti>

/*
 * Copyright (c) 2008
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 */

#include "alloc_perf_suite.h"

#include <exam/suite.h>
#include <iostream>

using namespace std;

int main( int, char ** )
{
  exam::trivial_time_logger tl( cout );

  exam::test_suite t( "alloc performance", 20 );

  t.set_logger( &tl );

  alloc_test alt;

  t.add( &alloc_test::alloc, alt, "malloc" );

  return t.girdle();
}

// -*- C++ -*- Time-stamp: <08/08/12 18:27:28 yeti>

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
  t.add( &alloc_test::alloc5000, alt, "malloc 5000" );
  t.add( &alloc_test::alloc_mix, alt, "malloc mix" );

  return t.girdle();
}

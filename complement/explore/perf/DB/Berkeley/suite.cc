// -*- C++ -*- Time-stamp: <08/08/13 13:26:45 yeti>

/*
 * Copyright (c) 2008
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 */

#include "sc_perf_suite.h"

#include <exam/suite.h>
#include <iostream>

using namespace std;

int main( int, char ** )
{
  exam::trivial_time_logger tl( cout );

  exam::test_suite t( "Berkeley DB performance", 20 );

  t.set_logger( &tl );

  sleepycat_test st;

  t.add( &sleepycat_test::hash_open, st, "Open DB with DB_HASH" );

  return t.girdle();
}

// -*- C++ -*- Time-stamp: <07/09/06 11:11:58 ptr>

/*
 *
 * Copyright (c) 2007
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include "sockios_perf_suite.h"
#include "sockios_perf.h"

#include <exam/suite.h>
#include <iostream>

using namespace std;

int EXAM_IMPL(sockios_perf_suite)
{
  exam::trivial_time_logger tlogger( std::cout );
  exam::test_suite t( "libsockios performance test suite", 10 );
  t.set_logger( &tlogger );

  sockios_perf p;

  t.add( &sockios_perf::exchange1, p, "client write 8K blocks" );
  t.add( &sockios_perf::exchange2, p, "client read 8K blocks" );
  t.add( &sockios_perf::exchange3, p, "client write/read 8K blocks" );

  return t.girdle();
}

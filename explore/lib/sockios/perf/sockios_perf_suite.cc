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

int EXAM_IMPL(sockios_perf_suite_b)
{
  exam::trivial_time_logger tlogger( std::cout );
  exam::test_suite t( "libsockios performance test suite 0", 10 );
  t.set_logger( &tlogger );

  sockios_perf_SrvR p;
  t.add( &sockios_perf_SrvR::r1, p, "client write 32 1638400 bytes blocks" );
  t.add( &sockios_perf_SrvR::r2, p, "client write 1024 51200 bytes blocks" );
  t.add( &sockios_perf_SrvR::r3, p, "client write 4096 12800 bytes blocks" );
  t.add( &sockios_perf_SrvR::r4, p, "client write 6400 8192 bytes blocks" );
  t.add( &sockios_perf_SrvR::r5, p, "client write 12800 4096 bytes blocks" );
  t.add( &sockios_perf_SrvR::r6, p, "client write 25600 2048 bytes blocks" );
  t.add( &sockios_perf_SrvR::r7, p, "client write 51200 1024 bytes blocks" );
  t.add( &sockios_perf_SrvR::r8, p, "client write 102400 512 bytes blocks" );
  t.add( &sockios_perf_SrvR::r9, p, "client write 204800 256 bytes blocks" );
  t.add( &sockios_perf_SrvR::r10, p, "client write 409600 128 bytes blocks" );
  t.add( &sockios_perf_SrvR::r11, p, "client write 819200 64 bytes blocks" );

  return t.girdle();
}

int EXAM_IMPL(sockios_perf_suite_c)
{
  exam::trivial_time_logger tlogger( std::cout );
  exam::test_suite t( "libsockios performance test suite 1", 10 );
  t.set_logger( &tlogger );

  sockios_perf_SrvW p;
  t.add( &sockios_perf_SrvW::r1, p, "client read 32 1638400 bytes blocks" );
  t.add( &sockios_perf_SrvW::r2, p, "client read 1024 51200 bytes blocks" );
  t.add( &sockios_perf_SrvW::r3, p, "client read 4096 12800 bytes blocks" );
  t.add( &sockios_perf_SrvW::r4, p, "client read 6400 8192 bytes blocks" );
  t.add( &sockios_perf_SrvW::r5, p, "client read 12800 4096 bytes blocks" );
  t.add( &sockios_perf_SrvW::r6, p, "client read 25600 2048 bytes blocks" );
  t.add( &sockios_perf_SrvW::r7, p, "client read 51200 1024 bytes blocks" );
  t.add( &sockios_perf_SrvW::r8, p, "client read 102400 512 bytes blocks" );
  t.add( &sockios_perf_SrvW::r9, p, "client read 204800 256 bytes blocks" );
  t.add( &sockios_perf_SrvW::r10, p, "client read 409600 128 bytes blocks" );
  t.add( &sockios_perf_SrvW::r11, p, "client read 819200 64 bytes blocks" );

  return t.girdle();
}

int EXAM_IMPL(sockios_perf_suite_d)
{
  exam::trivial_time_logger tlogger( std::cout );
  exam::test_suite t( "libsockios performance test suite 2", 10 );
  t.set_logger( &tlogger );

  sockios_perf_SrvRW p;
  t.add( &sockios_perf_SrvRW::r1, p, "client write/read 32 1638400 bytes blocks" );
  t.add( &sockios_perf_SrvRW::r2, p, "client write/read 1024 51200 bytes blocks" );
  t.add( &sockios_perf_SrvRW::r3, p, "client write/read 4096 12800 bytes blocks" );
  t.add( &sockios_perf_SrvRW::r4, p, "client write/read 6400 8192 bytes blocks" );
  t.add( &sockios_perf_SrvRW::r5, p, "client write/read 12800 4096 bytes blocks" );
  t.add( &sockios_perf_SrvRW::r6, p, "client write/read 25600 2048 bytes blocks" );
  t.add( &sockios_perf_SrvRW::r7, p, "client write/read 51200 1024 bytes blocks" );
  t.add( &sockios_perf_SrvRW::r8, p, "client write/read 102400 512 bytes blocks" );
  t.add( &sockios_perf_SrvRW::r9, p, "client write/read 204800 256 bytes blocks" );
  t.add( &sockios_perf_SrvRW::r10, p, "client write/read 409600 128 bytes blocks" );
  t.add( &sockios_perf_SrvRW::r11, p, "client write/read 819200 64 bytes blocks" );

  return t.girdle();
}

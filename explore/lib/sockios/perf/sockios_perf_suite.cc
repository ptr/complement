// -*- C++ -*- Time-stamp: <09/02/04 11:33:18 ptr>

/*
 *
 * Copyright (c) 2007, 2009
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include "sockios_perf_suite.h"
#include "sockios_perf.h"

#include <exam/suite.h>
#include <iostream>

#include <misc/opts.h>

using namespace std;

int main( int argc, const char** argv )
{
  Opts opts;

  opts.description( "test suite for 'xmt' framework" );
  opts.usage( "[options]" );

  opts << option<void>( "print this help message", 'h', "help" )
       << option<void>( "list all test cases", 'l', "list" )
       << option<std::string>( "run tests by number <num list>", 'r', "run" )
       << option<void>( "print status of tests within test suite", 'v', "verbose" )
       << option<void>(  "trace checks", 't', "trace" );

  try {
    opts.parse( argc, argv );
  }
  catch (...) {
    opts.help( std::cerr );
    return 1;
  }

  if ( opts.is_set( 'h' ) ) {
    opts.help( std::cerr );
    return 0;
  }

  exam::trivial_time_logger tlogger( std::cout );
  exam::test_suite t( "libsockios performance test suite 0", 5 );
  t.set_logger( &tlogger );

  exam::test_suite::test_case_type tc[30];

  sockios_perf_SrvR p;

  tc[0] = t.add( &sockios_perf_SrvR::rx<1,1,6480>, p, "client write 1 1 bytes blocks (reference)" );
  tc[1] = t.add( &sockios_perf_SrvR::rx<32,1638400,6480>, p, "client write 32 1638400 bytes blocks", tc[0] );
  tc[2] = t.add( &sockios_perf_SrvR::rx<1024,51200,6480>, p, "client write 1024 51200 bytes blocks", tc[1] );
  tc[3] = t.add( &sockios_perf_SrvR::rx<4096,12800,6480>, p, "client write 4096 12800 bytes blocks", tc[2] );
  tc[4] = t.add( &sockios_perf_SrvR::rx<6400,8192,6480>, p, "client write 6400 8192 bytes blocks", tc[3] );
  tc[5] = t.add( &sockios_perf_SrvR::rx<12800,4096,6480>, p, "client write 12800 4096 bytes blocks", tc[4] );
  tc[6] = t.add( &sockios_perf_SrvR::rx<25600,2048,6480>, p, "client write 25600 2048 bytes blocks", tc[5] );
  tc[7] = t.add( &sockios_perf_SrvR::rx<51200,1024,6480>, p, "client write 51200 1024 bytes blocks", tc[6] );
  tc[8] = t.add( &sockios_perf_SrvR::rx<102400,512,6480>, p, "client write 102400 512 bytes blocks", tc[7] );
  tc[9] = t.add( &sockios_perf_SrvR::rx<204800,256,6480>, p, "client write 204800 256 bytes blocks", tc[8] );
  tc[10] = t.add( &sockios_perf_SrvR::rx<409600,128,6480>, p, "client write 409600 128 bytes blocks", tc[9] );
  tc[11] = t.add( &sockios_perf_SrvR::rx<819200,64,6480>, p, "client write 819200 64 bytes blocks", tc[10] );

  sockios_perf_SrvW pw;

  tc[12] = t.add( &sockios_perf_SrvW::rx<1,1,6480>, pw, "client read 1 1 bytes blocks (reference)", tc[11] );
  tc[13] = t.add( &sockios_perf_SrvW::rx<32,1638400,6480>, pw, "client read 32 1638400 bytes blocks", tc[12] );
  tc[14] = t.add( &sockios_perf_SrvW::rx<1024,51200,6480>, pw, "client read 1024 51200 bytes blocks", tc[13] );
  tc[15] = t.add( &sockios_perf_SrvW::rx<4096,12800,6480>, pw, "client read 4096 12800 bytes blocks", tc[14] );
  tc[16] = t.add( &sockios_perf_SrvW::rx<6400,8192,6480>, pw, "client read 6400 8192 bytes blocks", tc[15] );
  tc[17] = t.add( &sockios_perf_SrvW::rx<12800,4096,6480>, pw, "client read 12800 4096 bytes blocks", tc[16] );
  tc[18] = t.add( &sockios_perf_SrvW::rx<25600,2048,6480>, pw, "client read 25600 2048 bytes blocks", tc[17] );
  tc[19] = t.add( &sockios_perf_SrvW::rx<51200,1024,6480>, pw, "client read 51200 1024 bytes blocks", tc[18] );
  tc[20] = t.add( &sockios_perf_SrvW::rx<102400,512,6480>, pw, "client read 102400 512 bytes blocks", tc[19] );
  tc[21] = t.add( &sockios_perf_SrvW::rx<204800,256,6480>, pw, "client read 204800 256 bytes blocks", tc[20] );
  tc[22] = t.add( &sockios_perf_SrvW::rx<409600,128,6480>, pw, "client read 409600 128 bytes blocks", tc[21] );
  tc[23] = t.add( &sockios_perf_SrvW::rx<819200,64,6480>, pw, "client read 819200 64 bytes blocks", tc[22] );

#if 1
  tc[36] = t.add( &sockios_perf_SrvR::rx<100,64,6480>, p, "client write 100 64 bytes blocks", tc[23] );
  tc[37] = t.add( &sockios_perf_SrvR::rx<100,128,6480>, p, "client write 100 128 bytes blocks", tc[36]  );
  tc[38] = t.add( &sockios_perf_SrvR::rx<100,256,6480>, p, "client write 100 256 bytes blocks", tc[37] );
  tc[39] = t.add( &sockios_perf_SrvR::rx<100,512,6480>, p, "client write 100 512 bytes blocks", tc[38] );
  tc[40] = t.add( &sockios_perf_SrvR::rx<100,1024,6480>, p, "client write 100 1024 bytes blocks", tc[39] );
  tc[41] = t.add( &sockios_perf_SrvR::rx<100,2048,6480>, p, "client write 100 2048 bytes blocks", tc[40] );
  tc[42] = t.add( &sockios_perf_SrvR::rx<100,4096,6480>, p, "client write 100 4096 bytes blocks", tc[41] );
  tc[43] = t.add( &sockios_perf_SrvR::rx<100,8192,6480>, p, "client write 100 8192 bytes blocks", tc[42] );
  tc[44] = t.add( &sockios_perf_SrvR::rx<100,16384,6480>, p, "client write 100 16384 bytes blocks", tc[43] );
  tc[45] = t.add( &sockios_perf_SrvR::rx<100,32768,6480>, p, "client write 100 32768 bytes blocks", tc[44] );
  tc[46] = t.add( &sockios_perf_SrvR::rx<100,65536,6480>, p, "client write 100 65536 bytes blocks", tc[45] );
  tc[47] = t.add( &sockios_perf_SrvR::rx<100,131072,6480>, p, "client write 100 131072 bytes blocks", tc[46] );
  tc[48] = t.add( &sockios_perf_SrvR::rx<100,262144,6480>, p, "client write 100 262144 bytes blocks", tc[47] );
#endif

  sockios_perf_SrvRW prw;

  tc[24] = t.add( &sockios_perf_SrvRW::rx<1,1,6480>, prw, "client write/read 1 1 bytes blocks (reference)", tc[48] );
  tc[25] = t.add( &sockios_perf_SrvRW::rx<32,1638400,6480>, prw, "client write/read 32 1638400 bytes blocks", tc[24] );
  tc[26] = t.add( &sockios_perf_SrvRW::rx<1024,51200,6480>, prw, "client write/read 1024 51200 bytes blocks", tc[25] );
#if 0
  tc[27] = t.add( &sockios_perf_SrvRW::rx<4096,12800,6480>, prw, "client write/read 4096 12800 bytes blocks", tc[26] );
  tc[28] = t.add( &sockios_perf_SrvRW::rx<6400,8192,6480>, prw, "client write/read 6400 8192 bytes blocks", tc[27] );
  tc[29] = t.add( &sockios_perf_SrvRW::rx<12800,4096,6480>, prw, "client write/read 12800 4096 bytes blocks", tc[28] );
  tc[30] = t.add( &sockios_perf_SrvRW::rx<25600,2048,6480>, prw, "client write/read 25600 2048 bytes blocks", tc[29] );
  tc[31] = t.add( &sockios_perf_SrvRW::rx<51200,1024,6480>, prw, "client write/read 51200 1024 bytes blocks", tc[30] );
  tc[32] = t.add( &sockios_perf_SrvRW::rx<102400,512,6480>, prw, "client write/read 102400 512 bytes blocks", tc[31] );
  tc[33] = t.add( &sockios_perf_SrvRW::rx<204800,256,6480>, prw, "client write/read 204800 256 bytes blocks", tc[32] );
  tc[34] = t.add( &sockios_perf_SrvRW::rx<409600,128,6480>, prw, "client write/read 409600 128 bytes blocks", tc[33] );
  tc[35] = t.add( &sockios_perf_SrvRW::rx<819200,64,6480>, prw, "client write/read 819200 64 bytes blocks", tc[34] );
#endif

  if ( opts.is_set( 'l' ) ) {
    t.print_graph( std::cerr );
    return 0;
  }

  if ( opts.is_set( 'v' ) ) {
    t.flags( t.flags() | exam::base_logger::verbose );
  }

  if ( opts.is_set( 't' ) ) {
    t.flags( t.flags() | exam::base_logger::trace );
  }

  if ( opts.is_set( 'r' ) ) {
    std::stringstream ss( opts.get<std::string>( 'r' ) );
    int n;
    while ( ss >> n ) {
      t.single( n );
    }

    return 0;
  }

  return t.girdle();
}


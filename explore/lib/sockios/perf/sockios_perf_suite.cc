// -*- C++ -*- Time-stamp: <2010-11-02 15:00:26 ptr>

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

  exam::test_suite::test_case_type tc[100];

  sockios_perf_SrvR p;

  tc[0] = t.add( &sockios_perf_SrvR::rx<1,1,6480,0>, p, "client write 1 1 bytes blocks (reference)" );
  tc[1] = t.add( &sockios_perf_SrvR::rx<32,1638400,6480,0>, p, "client write 32 1638400 bytes blocks", tc[0] );
  tc[2] = t.add( &sockios_perf_SrvR::rx<1024,51200,6480,0>, p, "client write 1024 51200 bytes blocks", tc[1] );
  tc[3] = t.add( &sockios_perf_SrvR::rx<4096,12800,6480,0>, p, "client write 4096 12800 bytes blocks", tc[2] );
  tc[4] = t.add( &sockios_perf_SrvR::rx<6400,8192,6480,0>, p, "client write 6400 8192 bytes blocks", tc[3] );
  tc[5] = t.add( &sockios_perf_SrvR::rx<12800,4096,6480,0>, p, "client write 12800 4096 bytes blocks", tc[4] );
  tc[6] = t.add( &sockios_perf_SrvR::rx<25600,2048,6480,0>, p, "client write 25600 2048 bytes blocks", tc[5] );
  tc[7] = t.add( &sockios_perf_SrvR::rx<51200,1024,6480,0>, p, "client write 51200 1024 bytes blocks", tc[6] );
  tc[8] = t.add( &sockios_perf_SrvR::rx<102400,512,6480,0>, p, "client write 102400 512 bytes blocks", tc[7] );
  tc[9] = t.add( &sockios_perf_SrvR::rx<204800,256,6480,0>, p, "client write 204800 256 bytes blocks", tc[8] );
  tc[10] = t.add( &sockios_perf_SrvR::rx<409600,128,6480,0>, p, "client write 409600 128 bytes blocks", tc[9] );
  tc[11] = t.add( &sockios_perf_SrvR::rx<819200,64,6480,0>, p, "client write 819200 64 bytes blocks", tc[10] );

  sockios_perf_SrvW pw;

  tc[12] = t.add( &sockios_perf_SrvW::rx<1,1,6480,0>, pw, "client read 1 1 bytes blocks (reference)", tc[11] );
  tc[13] = t.add( &sockios_perf_SrvW::rx<32,1638400,6480,0>, pw, "client read 32 1638400 bytes blocks", tc[12] );
  tc[14] = t.add( &sockios_perf_SrvW::rx<1024,51200,6480,0>, pw, "client read 1024 51200 bytes blocks", tc[13] );
  tc[15] = t.add( &sockios_perf_SrvW::rx<4096,12800,6480,0>, pw, "client read 4096 12800 bytes blocks", tc[14] );
  tc[16] = t.add( &sockios_perf_SrvW::rx<6400,8192,6480,0>, pw, "client read 6400 8192 bytes blocks", tc[15] );
  tc[17] = t.add( &sockios_perf_SrvW::rx<12800,4096,6480,0>, pw, "client read 12800 4096 bytes blocks", tc[16] );
  tc[18] = t.add( &sockios_perf_SrvW::rx<25600,2048,6480,0>, pw, "client read 25600 2048 bytes blocks", tc[17] );
  tc[19] = t.add( &sockios_perf_SrvW::rx<51200,1024,6480,0>, pw, "client read 51200 1024 bytes blocks", tc[18] );
  tc[20] = t.add( &sockios_perf_SrvW::rx<102400,512,6480,0>, pw, "client read 102400 512 bytes blocks", tc[19] );
  tc[21] = t.add( &sockios_perf_SrvW::rx<204800,256,6480,0>, pw, "client read 204800 256 bytes blocks", tc[20] );
  tc[22] = t.add( &sockios_perf_SrvW::rx<409600,128,6480,0>, pw, "client read 409600 128 bytes blocks", tc[21] );
  tc[23] = t.add( &sockios_perf_SrvW::rx<819200,64,6480,0>, pw, "client read 819200 64 bytes blocks", tc[22] );

  tc[36] = t.add( &sockios_perf_SrvR::rx<100,64,6480,0>, p, "client write 100 64 bytes blocks", tc[23] );
  tc[37] = t.add( &sockios_perf_SrvR::rx<100,128,6480,0>, p, "client write 100 128 bytes blocks", tc[36]  );
  tc[38] = t.add( &sockios_perf_SrvR::rx<100,256,6480,0>, p, "client write 100 256 bytes blocks", tc[37] );
  tc[39] = t.add( &sockios_perf_SrvR::rx<100,512,6480,0>, p, "client write 100 512 bytes blocks", tc[38] );
  tc[40] = t.add( &sockios_perf_SrvR::rx<100,1024,6480,0>, p, "client write 100 1024 bytes blocks", tc[39] );
  tc[41] = t.add( &sockios_perf_SrvR::rx<100,2048,6480,0>, p, "client write 100 2048 bytes blocks", tc[40] );
  tc[42] = t.add( &sockios_perf_SrvR::rx<100,4096,6480,0>, p, "client write 100 4096 bytes blocks", tc[41] );
  tc[43] = t.add( &sockios_perf_SrvR::rx<100,8192,6480,0>, p, "client write 100 8192 bytes blocks", tc[42] );
  tc[44] = t.add( &sockios_perf_SrvR::rx<100,16384,6480,0>, p, "client write 100 16384 bytes blocks", tc[43] );
  tc[45] = t.add( &sockios_perf_SrvR::rx<100,32768,6480,0>, p, "client write 100 32768 bytes blocks", tc[44] );
  tc[46] = t.add( &sockios_perf_SrvR::rx<100,65536,6480,0>, p, "client write 100 65536 bytes blocks", tc[45] );
  tc[47] = t.add( &sockios_perf_SrvR::rx<100,131072,6480,0>, p, "client write 100 131072 bytes blocks", tc[46] );
  tc[48] = t.add( &sockios_perf_SrvR::rx<100,262144,6480,0>, p, "client write 100 262144 bytes blocks", tc[47] );

  sockios_perf_SrvRW prw;

  tc[24] = t.add( &sockios_perf_SrvRW::rx<1,1,6480,0>, prw, "client write/read 1 1 bytes blocks (reference)", tc[48] );
  tc[25] = t.add( &sockios_perf_SrvRW::rx<32,1638400,6480,0>, prw, "client write/read 32 1638400 bytes blocks", tc[24] );
  tc[26] = t.add( &sockios_perf_SrvRW::rx<1024,51200,6480,0>, prw, "client write/read 1024 51200 bytes blocks", tc[25] );
#if 0
  tc[27] = t.add( &sockios_perf_SrvRW::rx<4096,12800,6480,0>, prw, "client write/read 4096 12800 bytes blocks", tc[26] );
  tc[28] = t.add( &sockios_perf_SrvRW::rx<6400,8192,6480,0>, prw, "client write/read 6400 8192 bytes blocks", tc[27] );
  tc[29] = t.add( &sockios_perf_SrvRW::rx<12800,4096,6480,0>, prw, "client write/read 12800 4096 bytes blocks", tc[28] );
  tc[30] = t.add( &sockios_perf_SrvRW::rx<25600,2048,6480,0>, prw, "client write/read 25600 2048 bytes blocks", tc[29] );
  tc[31] = t.add( &sockios_perf_SrvRW::rx<51200,1024,6480,0>, prw, "client write/read 51200 1024 bytes blocks", tc[30] );
  tc[32] = t.add( &sockios_perf_SrvRW::rx<102400,512,6480,0>, prw, "client write/read 102400 512 bytes blocks", tc[31] );
  tc[33] = t.add( &sockios_perf_SrvRW::rx<204800,256,6480,0>, prw, "client write/read 204800 256 bytes blocks", tc[32] );
  tc[34] = t.add( &sockios_perf_SrvRW::rx<409600,128,6480,0>, prw, "client write/read 409600 128 bytes blocks", tc[33] );
  tc[35] = t.add( &sockios_perf_SrvRW::rx<819200,64,6480,0>, prw, "client write/read 819200 64 bytes blocks", tc[34] );
#endif

  // TCP_NODELAY ...

  tc[50] = t.add( &sockios_perf_SrvR::rx<1,1,6480,1>, p, "client write 1 1 bytes blocks (reference) TCP_NODELAY" );
  tc[51] = t.add( &sockios_perf_SrvR::rx<32,1638400,6480,1>, p, "client write 32 1638400 bytes blocks TCP_NODELAY", tc[50] );
  tc[52] = t.add( &sockios_perf_SrvR::rx<1024,51200,6480,1>, p, "client write 1024 51200 bytes blocks TCP_NODELAY", tc[51] );
  tc[53] = t.add( &sockios_perf_SrvR::rx<4096,12800,6480,1>, p, "client write 4096 12800 bytes blocks TCP_NODELAY", tc[52] );
  tc[54] = t.add( &sockios_perf_SrvR::rx<6400,8192,6480,1>, p, "client write 6400 8192 bytes blocks TCP_NODELAY", tc[53] );
  tc[55] = t.add( &sockios_perf_SrvR::rx<12800,4096,6480,1>, p, "client write 12800 4096 bytes blocks TCP_NODELAY", tc[54] );
  tc[56] = t.add( &sockios_perf_SrvR::rx<25600,2048,6480,1>, p, "client write 25600 2048 bytes blocks TCP_NODELAY", tc[55] );
  tc[57] = t.add( &sockios_perf_SrvR::rx<51200,1024,6480,1>, p, "client write 51200 1024 bytes blocks TCP_NODELAY", tc[56] );
  tc[58] = t.add( &sockios_perf_SrvR::rx<102400,512,6480,1>, p, "client write 102400 512 bytes blocks TCP_NODELAY", tc[57] );
  tc[59] = t.add( &sockios_perf_SrvR::rx<204800,256,6480,1>, p, "client write 204800 256 bytes blocks TCP_NODELAY", tc[58] );
  tc[60] = t.add( &sockios_perf_SrvR::rx<409600,128,6480,1>, p, "client write 409600 128 bytes blocks TCP_NODELAY", tc[59] );
  tc[61] = t.add( &sockios_perf_SrvR::rx<819200,64,6480,1>, p, "client write 819200 64 bytes blocks TCP_NODELAY", tc[60] );

  tc[62] = t.add( &sockios_perf_SrvW::rx<1,1,6480,1>, pw, "client read 1 1 bytes blocks (reference) TCP_NODELAY", tc[61] );
  tc[63] = t.add( &sockios_perf_SrvW::rx<32,1638400,6480,1>, pw, "client read 32 1638400 bytes blocks TCP_NODELAY", tc[62] );
  tc[64] = t.add( &sockios_perf_SrvW::rx<1024,51200,6480,1>, pw, "client read 1024 51200 bytes blocks TCP_NODELAY", tc[63] );
  tc[65] = t.add( &sockios_perf_SrvW::rx<4096,12800,6480,1>, pw, "client read 4096 12800 bytes blocks TCP_NODELAY", tc[64] );
  tc[66] = t.add( &sockios_perf_SrvW::rx<6400,8192,6480,1>, pw, "client read 6400 8192 bytes blocks TCP_NODELAY", tc[65] );
  tc[67] = t.add( &sockios_perf_SrvW::rx<12800,4096,6480,1>, pw, "client read 12800 4096 bytes blocks TCP_NODELAY", tc[66] );
  tc[68] = t.add( &sockios_perf_SrvW::rx<25600,2048,6480,1>, pw, "client read 25600 2048 bytes blocks TCP_NODELAY", tc[67] );
  tc[69] = t.add( &sockios_perf_SrvW::rx<51200,1024,6480,1>, pw, "client read 51200 1024 bytes blocks TCP_NODELAY", tc[68] );
  tc[70] = t.add( &sockios_perf_SrvW::rx<102400,512,6480,1>, pw, "client read 102400 512 bytes blocks TCP_NODELAY", tc[69] );
  tc[71] = t.add( &sockios_perf_SrvW::rx<204800,256,6480,1>, pw, "client read 204800 256 bytes blocks TCP_NODELAY", tc[70] );
  tc[72] = t.add( &sockios_perf_SrvW::rx<409600,128,6480,1>, pw, "client read 409600 128 bytes blocks TCP_NODELAY", tc[71] );
  tc[73] = t.add( &sockios_perf_SrvW::rx<819200,64,6480,1>, pw, "client read 819200 64 bytes blocks TCP_NODELAY", tc[72] );

  tc[76] = t.add( &sockios_perf_SrvR::rx<100,64,6480,1>, p, "client write 100 64 bytes blocks TCP_NODELAY", tc[73] );
  tc[77] = t.add( &sockios_perf_SrvR::rx<100,128,6480,1>, p, "client write 100 128 bytes blocks TCP_NODELAY", tc[76]  );
  tc[78] = t.add( &sockios_perf_SrvR::rx<100,256,6480,1>, p, "client write 100 256 bytes blocks TCP_NODELAY", tc[77] );
  tc[79] = t.add( &sockios_perf_SrvR::rx<100,512,6480,1>, p, "client write 100 512 bytes blocks TCP_NODELAY", tc[78] );
  tc[80] = t.add( &sockios_perf_SrvR::rx<100,1024,6480,1>, p, "client write 100 1024 bytes blocks TCP_NODELAY", tc[79] );
  tc[81] = t.add( &sockios_perf_SrvR::rx<100,2048,6480,1>, p, "client write 100 2048 bytes blocks TCP_NODELAY", tc[80] );
  tc[82] = t.add( &sockios_perf_SrvR::rx<100,4096,6480,1>, p, "client write 100 4096 bytes blocks TCP_NODELAY", tc[81] );
  tc[83] = t.add( &sockios_perf_SrvR::rx<100,8192,6480,1>, p, "client write 100 8192 bytes blocks TCP_NODELAY", tc[82] );
  tc[84] = t.add( &sockios_perf_SrvR::rx<100,16384,6480,1>, p, "client write 100 16384 bytes blocks TCP_NODELAY", tc[83] );
  tc[85] = t.add( &sockios_perf_SrvR::rx<100,32768,6480,1>, p, "client write 100 32768 bytes blocks TCP_NODELAY", tc[84] );
  tc[86] = t.add( &sockios_perf_SrvR::rx<100,65536,6480,1>, p, "client write 100 65536 bytes blocks TCP_NODELAY", tc[85] );
  tc[87] = t.add( &sockios_perf_SrvR::rx<100,131072,6480,1>, p, "client write 100 131072 bytes blocks TCP_NODELAY", tc[86] );
  tc[88] = t.add( &sockios_perf_SrvR::rx<100,262144,6480,1>, p, "client write 100 262144 bytes blocks TCP_NODELAY", tc[87] );

  tc[89] = t.add( &sockios_perf_SrvRW::rx<1,1,6480,1>, prw, "client write/read 1 1 bytes blocks (reference) TCP_NODELAY" , tc[88] );
  tc[90] = t.add( &sockios_perf_SrvRW::rx<32,1638400,6480,1>, prw, "client write/read 32 1638400 bytes blocks TCP_NODELAY", tc[89] );
  tc[91] = t.add( &sockios_perf_SrvRW::rx<1024,51200,6480,1>, prw, "client write/read 1024 51200 bytes blocks TCP_NODELAY", tc[90] );

  sockios_perf_conn conn;

  t.add( &sockios_perf_conn::connect, conn, "connect" );
  t.add( &sockios_perf_conn::connect_basic, conn, "connect basic" );

  sockios_syslog_perf syslog_perf;

  t.add( &sockios_syslog_perf::syslog_mt<1000, 80, 8, sockios_syslog_perf::syslog_classic_worker>,
         syslog_perf, "syslog classic, 1000 messages, 80 length, 8 thread",
    t.add( &sockios_syslog_perf::syslog_mt<1000, 80, 1, sockios_syslog_perf::syslog_classic_worker>,
           syslog_perf, "syslog classic, 1000 messages, 80 length, 1 thread" ) );

  t.add( &sockios_syslog_perf::syslog_mt<1000, 80, 8, sockios_syslog_perf::syslog_dgram_worker>,
         syslog_perf, "syslog dgram, 1000 messages, 80 length, 8 thread",
    t.add( &sockios_syslog_perf::syslog_mt<1000, 80, 1, sockios_syslog_perf::syslog_dgram_worker>,
           syslog_perf, "syslog dgram, 1000 messages, 80 length, 1 thread" ) );

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


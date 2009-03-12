// -*- C++ -*- Time-stamp: <09/02/02 15:03:54 ptr>

/*
 *
 * Copyright (c) 2002, 2003, 2005-2009
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include "names.h"
#include "sockios_test.h"

#include <exam/suite.h>

#include <iostream>
#include <list>
#include <misc/opts.h>

using namespace std;

int EXAM_DECL(test_more_bytes_in_socket);
int EXAM_DECL(test_more_bytes_in_socket2);

int main( int argc, const char** argv )
{
  Opts opts;

  opts.description( "test suite for 'sockios' framework" );
  opts.usage( "[options]" );

  opts << option<void>( "print this help message", 'h', "help" )
       << option<void>( "list all test cases", 'l', "list" )
       << option<string>( "run tests by <numbers list>", 'r', "run" )
       << option<void>( "print status of tests within test suite", 'v', "verbose" )
       << option<void>(  "trace checks", 't', "trace" );

  try {
    opts.parse( argc, argv );
  }
  catch (...) {
    opts.help( cerr );
    return 1;
  }

  if ( opts.is_set( 'h' ) ) {
    opts.help( cerr );
    return 0;
  }

  exam::test_suite::test_case_type tc[6];

  exam::test_suite t( "libsockios test" );

  names_sockios_test names_test;

  t.add( &names_sockios_test::hostname_test, names_test, "names_sockios_test::hostname_test" );
  t.add( &names_sockios_test::service_test, names_test, "names_sockios_test::service_test" );
  t.add( &names_sockios_test::hostaddr_test1, names_test, "names_sockios_test::hostaddr_test1" );
  t.add( &names_sockios_test::hostaddr_test2, names_test, "names_sockios_test::hostaddr_test2" );
  t.add( &names_sockios_test::hostaddr_test3, names_test, "names_sockios_test::hostaddr_test3" );

  sockios_test test;

  tc[5] = t.add( &sockios_test::read0, test, "sockios2_test::read0",
    t.add( &sockios_test::srv_sigpipe, test, "sockios2_test::srv_sigpipe",
      tc[4] = t.add( &sockios_test::fork, test, "sockios2_test::fork",
        tc[3] = t.add( &sockios_test::processor_core_income_data, test, "all data available after sockstream was closed",
          t.add( &sockios_test::processor_core_getline, test, "check income data before sockstream was closed",
            t.add( &sockios_test::processor_core_two_local, test, "two local connects to connection processor",
              t.add( &sockios_test::processor_core_one_local, test, "one local connect to connection processor",
                t.add( &sockios_test::connect_disconnect, test, "sockios2_test::connect_disconnect",
                  tc[0] = t.add( &sockios_test::srv_core, test, "sockios2_test::srv_core" ) ) ) ) ) ) ) ) );

  t.add( &sockios_test::two_ports, test, "two servers", tc[0] );

  t.add( &sockios_test::disconnect_rawclnt, test, "disconnect raw client", 
    t.add( &sockios_test::disconnect, test, "disconnect sockstream", tc[3] ) );
  t.add( &sockios_test::income_data, test, "all data available after sockstream was closed, different processes", tc[4] );
  t.add( &sockios_test::few_packets_loop, test, "packets boundary, loop", 
    t.add( &sockios_test::few_packets, test, "packets boundary", tc[3] ) );

  exam::test_suite::test_case_type extratc[5];

  extratc[0] = tc[5];

  extratc[1] = t.add( &sockios_test::service_stop, test, "stop service", tc[4] );
  
  t.add( &sockios_test::ugly_echo, test, "ugly echo service",
    t.add( &sockios_test::echo, test, "echo service",
      t.add( &sockios_test::quants_reader, test, "read a few fixed-size data", extratc, extratc + 2 ) ) );

  if ( opts.is_set( 'l' ) ) {
    t.print_graph( cerr );
    return 0;
  }

  if ( opts.is_set( 'v' ) ) {
    t.flags( t.flags() | exam::base_logger::verbose );
  }

  if ( opts.is_set( 't' ) ) {
    t.flags( t.flags() | exam::base_logger::trace );
  }

  if ( opts.is_set( 'r' ) ) {
    stringstream ss( opts.get<string>( 'r' ) );
    int n;
    while ( ss >> n ) {
      t.single( n );
    }

    return 0;
  }

  return t.girdle();
}


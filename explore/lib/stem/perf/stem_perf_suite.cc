// -*- C++ -*- Time-stamp: <09/02/04 19:27:36 ptr>

/*
 *
 * Copyright (c) 2009
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include "stem_perf.h"

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
  exam::test_suite t( "libstem performance test suite 0", 10 );
  t.set_logger( &tlogger );

  exam::test_suite::test_case_type tc[30];

  stem_perf p;

  t.add( &stem_perf::net_loopback_inv2, p, "StEM event, via net loopback iface, inverted send",
    t.add( &stem_perf::net_loopback_inv, p, "StEM event, via net loopback iface, inverted",
      t.add( &stem_perf::net_loopback, p, "StEM event, via net loopback iface",
        t.add( &stem_perf::local_too, p, "StEM local event, with NetTransport",
          t.add( &stem_perf::local, p, "StEM local event" ) ) ) ) );

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


// -*- C++ -*- Time-stamp: <09/07/31 13:51:25 ptr>

/*
 *
 * Copyright (c) 2009
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include "janus_perf.h"

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
  exam::test_suite t( "janus performance test suite 0", 20 );
  t.set_logger( &tlogger );

  exam::test_suite::test_case_type tc[30];

  janus_perf p;

  t.add( &janus_perf::group_send_mt<10,100>, p, "group send mt: 10 users, 1000 msgs total",
    t.add( &janus_perf::group_send_mt<9,111>, p, "group send mt: 9 users, 1000 msgs total",
      t.add( &janus_perf::group_send_mt<8,125>, p, "group send mt: 8 users, 1000 msgs total",
        t.add( &janus_perf::group_send_mt<7,142>, p, "group send mt: 7 users, 1000 msgs total",
          t.add( &janus_perf::group_send_mt<6,166>, p, "group send mt: 6 users, 1000 msgs total",
            t.add( &janus_perf::group_send_mt<5,200>, p, "group send mt: 5 users, 1000 msgs total",
              t.add( &janus_perf::group_send_mt<4,250>, p, "group send mt: 4 users, 1000 msgs total",
                t.add( &janus_perf::group_send_mt<3,333>, p, "group send mt: 3 users, 1000 msgs total",
                  t.add( &janus_perf::group_send_mt<2,500>, p, "group send mt: 2 users, 1000 msgs total",
                    t.add( &janus_perf::group_send_mt<1,1000>, p, "group send mt: 1 users, 1000 msgs total" ) ) ) ) ) ) ) ) ) );

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


// -*- C++ -*- Time-stamp: <10/05/13 11:31:27 ptr>

/*
 *
 * Copyright (c) 2010
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include "yard_perf.h"

#include <exam/suite.h>
#include <iostream>

#include <misc/opts.h>

using namespace std;

int main( int argc, const char** argv )
{
  Opts opts;

  opts.description( "test suite for 'yard' framework" );
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
  exam::test_suite t( "libyard performance test suite 0", 20 );
  t.set_logger( &tlogger );

  exam::test_suite::test_case_type tc[30];

  yard_perf p;

  tc[1] = t.add( &yard_perf::put_more_more, p, "put-102400",
    t.add( &yard_perf::put_more, p, "put-10240",
      tc[0] = t.add( &yard_perf::put, p, "put-1024" ) ) );

  t.add( &yard_perf::put_get, p, "put/get-1024", tc[0] );
  t.add( &yard_perf::put_object_r2, p, "put object-1024 r2",
    t.add( &yard_perf::put_object, p, "put object-1024", tc[0] ) );

  t.add( &yard_perf::put_mess, p, "put message 1024" );

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


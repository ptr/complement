// -*- C++ -*- Time-stamp: <09/01/15 01:16:44 ptr>

/*
 *
 * Copyright (c) 2007, 2009
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include <config/feature.h>

#include "http_test_suite.h"
#include "http_test.h"

#include <misc/opts.h>

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

  exam::test_suite t( "http test" );

  http_test test;

  t.add( &http_test::header_io, test, "" );
  t.add( &http_test::header_sp, test, "" );
  t.add( &http_test::command, test, "" );
  t.add( &http_test::base_response, test, "" );
  t.add( &http_test::request, test, "" );
  t.add( &http_test::response, test, "" );

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

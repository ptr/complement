// -*- C++ -*- Time-stamp: <2011-04-21 13:18:52 ptr>

/*
 * Copyright (c) 2007, 2008, 2011
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#include <exam/suite.h>
#include <config/feature.h>

#include "misc_test.h"
#include <misc/opts.h>

extern void misc_test_suite_init( exam::test_suite&, misc_test& );
extern void chrono_test_suite_init( exam::test_suite&, chrono_test& );

int main( int argc, const char** argv )
{
  Opts opts;

  opts.description( "test suite for 'misc options' framework" );
  opts.usage( "[options]" );

  opts << option<void>( "print this help message", 'h', "help" )
       << option<void>( "list all test cases", 'l', "list" )
       << option<std::string>( "run tests by number", 'r', "run" )
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

  exam::test_suite t( "libmisc type_traits test" );

  misc_test test;
  chrono_test chr;

  misc_test_suite_init( t, test );
  chrono_test_suite_init( t, chr );

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

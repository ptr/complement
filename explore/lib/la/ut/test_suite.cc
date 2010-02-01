// -*- C++ -*- Time-stamp: <10/01/22 16:34:54 yeti>

/*
 * Copyright (c) 2008, 2009
 * Yandex LLC
 *
 */

#include <exam/suite.h>
#include <config/feature.h>
#include <misc/opts.h>

#include <iostream>
#include <sstream>

#include "la_test.h"

using namespace std;

int main( int argc, const char** argv )
{
  Opts opts;

  opts << option<void>( "print this help message", 'h', "help" )
       << option<void>( "list all test cases", 'l', "list" )
       << option<string>( "run tests by number", 'r', "run" )["0"]
       << option<void>( "print status of tests within test suite", 'v', "verbose" )
       << option<void>( "trace checks", 't', "trace" );

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

  exam::test_suite t( "Long Arithmetics test suite" );
  //exam::test_suite::test_case_type tc[1];

  la_test test;

  
  t.add( &la_test::boundary_32bit, test, "operations on 32 bit boundary",
    t.add( &la_test::two_chars, test, "two chars",
      t.add( &la_test::one_short, test, "one short",
        t.add( &la_test::one_char, test, "one char" ) ) ) );
      
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


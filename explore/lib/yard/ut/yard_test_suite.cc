// -*- C++ -*- Time-stamp: <2010-12-20 13:41:38 ptr>

/*
 * Copyright (c) 2010
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include <exam/suite.h>
#include <misc/opts.h>
#include "yard_test.h"

using namespace std;
using namespace std::tr2;

int main( int argc, const char** argv )
{
  exam::test_suite::test_case_type tc[7];

  exam::test_suite t( "libyard test suite" );
  yard_test test;

  t.add( &yard_test::manifest, test, "manifest",
    t.add( &yard_test::put_object, test, "put object",
      t.add( &yard_test::put, test, "put revision",
        t.add( &yard_test::create, test, "create hash" ) ) ) );

  t.add( &yard_test::linear_commits_neg, test, "non-linear commits in yard ng",
    tc[0] = t.add( &yard_test::linear_commits, test, "linear commits in yard ng",
      t.add( &yard_test::access, test, "access to yard ng",
        tc[2] = t.add( &yard_test::revision_in_memory, test, "revision in memory, yard ng" ) ) ) );

  t.add( &yard_test::fast_merge4, test, "fast merge with common ancestor in yard ng",
    t.add( &yard_test::fast_merge3, test, "fast merge right add yard ng",
      t.add( &yard_test::fast_merge2, test, "fast merge left add in yard ng",
        t.add( &yard_test::fast_merge1, test, "fast merge add different in yard ng",
          tc[1] = t.add( &yard_test::diff, test, "diff between commits in yard ng", tc[0] ) ) ) ) );

  t.add( &yard_test::fast_merge_conflict1, test, "fast merge conflict in yard ng", tc[1] );
  t.add( &yard_test::heads, test, "heads of commits graph in yard ng", tc[2] );
  t.add( &yard_test::merge1, test, "merge with conflict in yard ng", tc[1] );

  Opts opts;

  opts.description( "test suite for 'yard' framework" );
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

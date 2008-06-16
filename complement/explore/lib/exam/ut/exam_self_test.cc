// -*- C++ -*- Time-stamp: <07/09/01 09:10:26 ptr>

/*
 * Copyright (c) 2007
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 */

#include "exam_test_suite.h"
#include "misc/opts.h"
#include <vector>
#include <string>

using namespace std;

int main( int argc,const char** argv)
{
  // exam::test_suite t( "exam self test" );
  // t.add( exam_self_test, "exam self test suite" );
  //
  // return t.girdle();

  // return exam_self_test(0);
  
  exam::test_suite t( "exam self test" );
  exam_basic_test exam_basic;

  exam::test_suite::test_case_type d0 = t.add( &exam_basic_test::function_good, exam_basic, "call test, good calls" );
  t.add( &exam_basic_test::function, exam_basic, "call test, fail calls", d0 );
  exam::test_suite::test_case_type d = t.add( &exam_basic_test::dep, exam_basic, "call test, tests dependency", d0 );
  t.add( &exam_basic_test::trace, exam_basic, "trace flags test", d );
  t.add( &exam_basic_test::dep_test_suite, exam_basic, "test suites grouping", d );
  exam::test_suite::test_case_type d2 = t.add( &exam_basic_test::multiple_dep, exam_basic, "multiple dependencies", d );
  t.add( &exam_basic_test::multiple_dep_complex, exam_basic, "complex multiple dependencies", d2 );

  t.add( &exam_basic_test::perf, exam_basic, "performance timer test", d0 );
  t.add( &exam_basic_test::dry, exam_basic, "complex multiple dependencies, dry run", d2 );
  t.add( &exam_basic_test::single, exam_basic, "complex multiple dependencies, single test", d2 );

  Opts opts;

  opts.addflag( 'h', "help", "print this help message" );
  opts.addflag( 'l', "list", "list all test cases" );
  opts.add( 'r', 0, "run", "run tests by number" );
  opts.addflag( 'v', "verbose", "print status of tests within test suite" );
  opts.addflag( 't', "trace", "trace checks" );

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
    stringstream ss( opts.get( 'r' ) );
    int n;
    while ( ss >> n ) {
      t.single( n );
    }

    return 0;
  }

  return t.girdle();
}


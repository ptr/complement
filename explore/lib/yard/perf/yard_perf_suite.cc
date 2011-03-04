// -*- C++ -*- Time-stamp: <2011-03-04 18:47:29 ptr>

/*
 *
 * Copyright (c) 2010-2011
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

  t.add( &yard_perf::unpacking, p, "load from file a block 10^4 times",
    t.add( &yard_perf::packing, p, "save to file a block 10^4 times") );

  t.add(&yard_perf::consecutive_insert, p, "consecutive insert (10^5 entires)");
  t.add(&yard_perf::consecutive_insert_big, p, "consecutive insert (10^6 entires)");
  t.add(&yard_perf::random_insert_big, p, "random insert (10^6 entires)");
  t.add(&yard_perf::consecutive_insert_with_data, p, "consecutive insert with data (n entires)");
  t.add(&yard_perf::random_insert_with_data, p, "random insert with data (n entires)");
  t.add(&yard_perf::multiple_files, p, "multiple files");
  t.add(&yard_perf::random_lookup, p, "random lookup");

  t.add( &yard_perf::mess, p, "put message 1024" );
  t.add( &yard_perf::put_revisions, p, "put blob 1024 [revision]" );
  t.add( &yard_perf::mess_insert, p, "put 1000 new messages" );
  t.add( &yard_perf::mess_insert_single_commit, p, "put 1000 new messages in one transaction" );

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


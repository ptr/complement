// -*- C++ -*- Time-stamp: <2011-02-10 16:33:46 ptr>

/*
 * Copyright (c) 2010-2011
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

  t.add( &yard_test::pack_test, test, "pack test");

  t.add( &yard_test::pack_unpack, test, "pack-unpack test",
    t.add( &yard_test::block_type_divide, test, "block route divide test",
      t.add( &yard_test::block_type_route, test, "block route test",
        t.add( &yard_test::block_type_lookup, test, "block lookup test"
    ) ) ) );

  t.add( &yard_test::btree_random, test, "Btree random test",
    t.add( &yard_test::btree_basic, test, "BTree test" ) );

  t.add( &yard_test::btree_init_existed, test, "Btree init existed test");

  t.add( &yard_test::linear_commits_neg, test, "non-linear commits",
    tc[0] = t.add( &yard_test::linear_commits, test, "linear commits",
      t.add( &yard_test::access, test, "access",
        tc[2] = t.add( &yard_test::revision_in_memory, test, "revision in memory" ) ) ) );

  tc[3] = t.add( &yard_test::diff_from_revision, test, "recover delta from revision",
    t.add( &yard_test::manifest_from_revision, test, "recover manifest from revision", tc[2] ) );

  t.add( &yard_test::commit_from_revision1, test, "recover commit node from revision (manifest)", tc[2] );
  t.add( &yard_test::commit_from_revision2, test, "recover commit node from revision (delta)", tc[3] );

  t.add( &yard_test::fast_merge4, test, "fast merge with common ancestor",
    t.add( &yard_test::fast_merge3, test, "fast merge right add",
      t.add( &yard_test::fast_merge2, test, "fast merge left add",
        t.add( &yard_test::fast_merge1, test, "fast merge add different",
          tc[1] = t.add( &yard_test::diff, test, "diff between commits", tc[0] ) ) ) ) );

  t.add( &yard_test::fast_merge_conflict1, test, "fast merge conflict", tc[1] );
  t.add( &yard_test::heads, test, "heads of commits graph", tc[2] );
  t.add( &yard_test::merge1, test, "merge with conflict", tc[1] );

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

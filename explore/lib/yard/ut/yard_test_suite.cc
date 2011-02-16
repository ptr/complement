// -*- C++ -*- Time-stamp: <2011-02-16 17:39:16 ptr>

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
  yard_block_test block_test;
  yard_btree_test btree_test;
  yard_test test;

  tc[4] = t.add( &yard_test::pack_test, test, "pack test");

  tc[5] = t.add( &yard_block_test::pack_unpack, block_test, "pack-unpack test",
    t.add( &yard_block_test::block_type_divide, block_test, "block route divide test",
      t.add( &yard_block_test::block_type_route, block_test, "block route test",
        t.add( &yard_block_test::block_type_lookup, block_test, "block lookup test", tc[4] ))));

  t.add( &yard_btree_test::bad_key, btree_test, "Btree bad key",
    t.add( &yard_btree_test::insert_extract, btree_test, "Btree extract",
      t.add( &yard_btree_test::btree_random, btree_test, "Btree random test",
        t.add( &yard_btree_test::btree_basic, btree_test, "BTree test", tc[5] ) ) ) );

  t.add( &yard_btree_test::open_modes, btree_test, "Btree open modes", tc[5] );

  t.add( &yard_btree_test::btree_init_existed, btree_test, "Btree init existed test");

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

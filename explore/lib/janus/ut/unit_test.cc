// -*- C++ -*- Time-stamp: <09/10/13 17:24:53 ptr>

/*
 *
 * Copyright (c) 2008-2009
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include "vt_operations.h"

#include <exam/suite.h>
#include <misc/opts.h>
#include <iostream>
#include <string>

int main( int argc, const char ** argv )
{
  using namespace std;

  Opts opts;

  opts << option<void>( "print this help message", 'h', "help" )
       << option<void>( "list all test cases", 'l', "list" )
       << option<string>( "run tests <sequence>", 'r', "run" )
       << option<void>( "print status of tests within test suite", 'v', "verbose" )
       << option<void>( "trace checks", 't', "trace" );

  try {
    opts.parse( argc, argv );
  }
  catch ( ... ) {
    opts.help( cerr );
    return 1;
  }

  if ( opts.is_set( 'h' ) ) {
    opts.help( cerr );
    return 0;
  }

  using namespace janus;

  exam::test_suite::test_case_type tc[10];

  exam::test_suite t( "virtual time operations" );

  janus::vtime_operations vt_oper;

  tc[3] = t.add( &vtime_operations::vt_max, vt_oper, "Max",
    tc[1] = t.add( &vtime_operations::vt_add, vt_oper, "Additions",
      tc[0] = t.add( &vtime_operations::vt_compare, vt_oper, "Compare" ) ) );
  t.add( &vtime_operations::vt_diff, vt_oper, "Differences", tc[0] );
  tc[2] = t.add( &vtime_operations::gvt_add, vt_oper, "Grouped VT additions", tc[1] );

  tc[5] = t.add( &vtime_operations::VT_one_group_late_replay, vt_oper, "VT one group late replay",
    t.add( &vtime_operations::VT_one_group_replay, vt_oper, "VT one group replay",
      t.add( &vtime_operations::VT_one_group_send, vt_oper, "VT one group send",
        t.add( &vtime_operations::VT_one_group_core3, vt_oper, "VT one group add third group member",
          tc[4] = t.add( &vtime_operations::VT_one_group_core, vt_oper, "VT one group add group member", tc[3] ) ) ) ) );

  t.add( &vtime_operations::VT_one_group_access_point, vt_oper, "VT network access points",
    t.add( &vtime_operations::VT_one_group_network, vt_oper, "VT over network", tc[4] ) );

  t.add( &vtime_operations::VT_one_group_recover, vt_oper, "VT one group recover", tc[5] );

  if ( opts.is_set( 'v' ) ) {
    t.flags( t.flags() | exam::base_logger::verbose );
  }

  if ( opts.is_set( 't' ) ) {
    t.flags( t.flags() | exam::base_logger::trace );
  }

  if ( opts.is_set( 'l' ) ) {
    t.print_graph( cerr );
    return 0;
  } else if ( opts.is_set( 'r' ) ) {
    stringstream ss( opts.get<string>( 'r' ) );
    int n;
    int res = 0;
    while ( ss >> n ) {
      res |= t.single( n );
    }

    return res;
  }
  
  return t.girdle();
}

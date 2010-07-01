// -*- C++ -*- Time-stamp: <10/06/10 16:12:21 ptr>

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

  exam::test_suite::test_case_type tc[6];

  exam::test_suite t( "virtual time operations" );

  janus::vtime_operations vt_oper;

  tc[0] = t.add( &vtime_operations::join_flush_exit, vt_oper, "VT join/flush/exit", 
            t.add( &vtime_operations::flush_and_exit, vt_oper, "VT flush and exit", 
              t.add( &vtime_operations::flush_and_join, vt_oper, "VT flush and join", 
                t.add( &vtime_operations::double_exit, vt_oper, "VT double exit",
                  t.add( &vtime_operations::double_flush, vt_oper, "VT double flush",
                    t.add( &vtime_operations::VT_one_group_join_exit, vt_oper, "VT one group join and exit",
                      t.add( &vtime_operations::VT_one_group_core3_sim, vt_oper, "VT one group add two members simultaneously",
                        t.add( &vtime_operations::VT_one_group_core3, vt_oper, "VT one group add third group member",
                          t.add( &vtime_operations::VT_one_group_core, vt_oper, "VT one group add group member" ) ) ) ) ) ) ) ) );

  tc[1] = t.add( &vtime_operations::VT_one_group_multiple_send, vt_oper, "VT one group multiple send",
            t.add( &vtime_operations::VT_one_group_send, vt_oper, "VT one group send", tc[0] ) );

  tc[2] = t.add( &vtime_operations::VT_one_group_late_replay, vt_oper, "VT one group late replay",
            t.add( &vtime_operations::VT_one_group_replay, vt_oper, "VT one group replay", tc[1] ) );

  tc[3] = t.add( &vtime_operations::VT_one_group_recover, vt_oper, "VT one group recover",
            t.add( &vtime_operations::VT_one_group_multiple_join_send, vt_oper, "VT one group multiple join/send",
              t.add( &vtime_operations::VT_one_group_multiple_joins, vt_oper, "VT one group multiple joins",
                t.add( &vtime_operations::VT_one_group_join_send, vt_oper, "VT one group join and send", tc[2] ) ) ) );

  tc[4] = t.add( &vtime_operations::VT_one_group_access_point, vt_oper, "VT network access points",
    t.add( &vtime_operations::VT_one_group_network, vt_oper, "VT over network", tc[1] ) );

  tc[5] = t.add( &vtime_operations::leader_recovery, vt_oper, "VT total order, leader recovery",
            t.add( &vtime_operations::leader_fail, vt_oper, "VT total order, leader fail",
              t.add( &vtime_operations::leader, vt_oper, "VT total order, leader", tc[4] ) ) );

  t.add( &vtime_operations::lock_and_torder, vt_oper, "VT total order, lock", tc[5] );

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

// -*- C++ -*- Time-stamp: <09/09/09 15:09:01 ptr>

#include "vt_operations.h"

#include <exam/suite.h>
#include <misc/opts.h>
#include <iostream>
#include <string>

// int EXAM_DECL(vtime_test_suite);

#if 0
int EXAM_IMPL(vtime_test_suite)
{
  using namespace janus;

  exam::test_suite::test_case_type tc[4];

  exam::test_suite t( "virtual time operations" );

  janus::vtime_operations vt_oper;

  t.flags( /* exam::base_logger::trace */ exam::base_logger::verbose );

  t.add( &vtime_operations::vt_max, vt_oper, "Max",
         tc[1] = t.add( &vtime_operations::vt_add, vt_oper, "Additions",
                        tc[0] = t.add( &vtime_operations::vt_compare, vt_oper, "Compare" ) ) );
  t.add( &vtime_operations::vt_diff, vt_oper, "Differences", tc[0] );
  t.add( &vtime_operations::gvt_add, vt_oper, "Grouped VT additions", tc[1] );

#if 0
  t.add( &vtime_operations::VTMess_core, vt_oper, "VTmess core transfer", 
         tc[2] = t.add( &vtime_operations::gvt_add, vt_oper, "Group VT add", tc[1] ) );

  t.add( &vtime_operations::mgroups, vt_oper, "mgroups",
    tc[3] = t.add( &vtime_operations::remote, vt_oper, "remote",
      t.add( &vtime_operations::VTEntryIntoGroup3, vt_oper, "VTEntryIntoGroup3",
        t.add( &vtime_operations::VTEntryIntoGroup2, vt_oper, "VTEntryIntoGroup2",
          t.add( &vtime_operations::VTEntryIntoGroup, vt_oper, "VTEntryIntoGroup",
            t.add( &vtime_operations::VTSubscription, vt_oper, "VTSubscription",
              t.add( &vtime_operations::VTDispatch2, vt_oper, "VTHandler2",
                t.add( &vtime_operations::VTDispatch2, vt_oper, "VTHandler1",
                  t.add( &vtime_operations::VTDispatch2, vt_oper, "VTDispatch2",
                    t.add( &vtime_operations::VTDispatch1, vt_oper, "VTDispatch1",
                      t.add( &vtime_operations::vt_object, vt_oper, "VT order", tc[2] )))))))))) );

  t.add( &vtime_operations::wellknownhost, vt_oper, "well-known host", tc[3] );
#endif

  return t.girdle();
}
#endif

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

  exam::test_suite::test_case_type tc[4];

  exam::test_suite t( "virtual time operations" );

  janus::vtime_operations vt_oper;

  t.flags( /* exam::base_logger::trace */ exam::base_logger::verbose );

  tc[3] = t.add( &vtime_operations::vt_max, vt_oper, "Max",
         tc[1] = t.add( &vtime_operations::vt_add, vt_oper, "Additions",
                        tc[0] = t.add( &vtime_operations::vt_compare, vt_oper, "Compare" ) ) );
  t.add( &vtime_operations::vt_diff, vt_oper, "Differences", tc[0] );
  tc[2] = t.add( &vtime_operations::gvt_add, vt_oper, "Grouped VT additions", tc[1] );
  t.add( &vtime_operations::VTMess_core, vt_oper, "VTmess core transfer", tc[2] );

  t.add( &vtime_operations::VT_one_group_send, vt_oper, "VT one group send",
    t.add( &vtime_operations::VT_one_group_core3, vt_oper, "VT one group add third group member",
       t.add( &vtime_operations::VT_one_group_core, vt_oper, "VT one group add group member", tc[3] ) ) );

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

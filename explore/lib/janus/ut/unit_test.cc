// -*- C++ -*- Time-stamp: <09/07/22 09:48:51 ptr>

#include "vt_operations.h"

int EXAM_DECL(vtime_test_suite);

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

int main( int, char ** )
{

  return vtime_test_suite(0);
}

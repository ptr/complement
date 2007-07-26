// -*- C++ -*- Time-stamp: <07/07/25 22:14:50 ptr>

#include "vt_operations.h"

int EXAM_DECL(vtime_test_suite);

int EXAM_IMPL(vtime_test_suite)
{
  exam::test_suite::test_case_type tc[3];

  exam::test_suite t( "virtual time operations" );

  vtime_operations vt_oper;

  t.add( &vtime_operations::vt_max, vt_oper, "Max",
         tc[1] = t.add( &vtime_operations::vt_add, vt_oper, "Additions",
                        tc[0] = t.add( &vtime_operations::vt_compare, vt_oper, "Compare" ) ) );
  t.add( &vtime_operations::vt_diff, vt_oper, "Differences", tc[0] );

  t.add( &vtime_operations::VTMess_core, vt_oper, "VTmess core transfer", 
         tc[2] = t.add( &vtime_operations::gvt_add, vt_oper, "Group VT add", tc[1] ) );

  t.add( &vtime_operations::VTDispatch, vt_oper, "VTDispatch",
         t.add( &vtime_operations::vt_object, vt_oper, "VT order", tc[2] ) );

  return t.girdle();
}

int main( int, char ** )
{

  return vtime_test_suite(0);
}

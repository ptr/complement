// -*- C++ -*- Time-stamp: <08/04/25 12:01:55 yeti>

#include "my_test_suite.h"
#include "my_test.h"

#include <config/feature.h>

int EXAM_IMPL(my_test_suite)
{
  exam::test_suite t( "test suite for SMTP server" );
  my_test test;

  // exam::test_suite::test_case_type tc0;


  t.add( &my_test::thread_call, test, "my_test::thread_call",
    t.add( &my_test::test_gen, test, "my_test::test_gen" ) );

#if 0
  tc0 = t.add( &my_test::test_gen, test, "my_test::test_gen" );
  t.add( &my_test::thread_call, test, "my_test::thread_call", tc0 );
  t.add( &my_test::thread_call, test, "my_test::thread_call_yet_more", tc0 );
#endif

  return t.girdle();
};

// -*- C++ -*- Time-stamp: <08/03/26 10:12:21 ptr>

#include "my_test_suite.h"
#include "my_test.h"

#include <config/feature.h>

int EXAM_IMPL(my_test_suite)
{
  exam::test_suite t( "my test" );
  my_test test;

  t.add( &my_test::test_gen, test, "my_test::test_gen" );
  t.add( &my_test::thread_call, test, "my_test::thread_call" );

  return t.girdle();
};

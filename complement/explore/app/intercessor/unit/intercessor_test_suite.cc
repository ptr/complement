// -*- C++ -*- Time-stamp: <07/10/22 18:19:19 yeti>

/*
 *
 * Copyright (c) 2007
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include "intercessor_test_suite.h"
#include "intercessor_test.h"

int EXAM_IMPL(intercessor_test_suite)
{
  exam::test_suite t( "intercessor test" );

  intercessor_test test;

  t.add( &intercessor_test::base, test, "intercessor_test::base" );
  t.add( &intercessor_test::processor, test, "intercessor_test::base" );
  t.add( &intercessor_test::processor_post, test, "intercessor_test::base" );
  // t.add( &intercessor::processor_external_post, test, "intercessor_test::base" );
  t.add( &intercessor_test::negative, test, "intercessor_test::base" );

  return t.girdle();
}

// -*- C++ -*- Time-stamp: <07/10/22 18:19:19 yeti>

/*
 *
 * Copyright (c) 2007
 * Petr Ovtchenkov
 *
 */

#include "intercessor_test_suite.h"
#include "http_test.h"
#include "intercessor_test.h"

int EXAM_IMPL(http_test_suite)
{
  exam::test_suite t( "http test" );

  http_test test;

  t.add( &http_test::header_io, test, "" );
  t.add( &http_test::header_sp, test, "" );
  t.add( &http_test::command, test, "" );
  t.add( &http_test::base_response, test, "" );
  t.add( &http_test::request, test, "" );
  t.add( &http_test::response, test, "" );

  return t.girdle();
}

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

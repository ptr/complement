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
#include "http_test.h"

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

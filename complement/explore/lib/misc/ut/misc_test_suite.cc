// -*- C++ -*- Time-stamp: <07/07/17 10:20:08 ptr>

/*
 * Copyright (c) 2007
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#include "misc_test_suite.h"
#include "misc_test.h"

#include <config/feature.h>

int EXAM_IMPL(misc_test_suite)
{
  exam::test_suite t( "libmisc? test" );
  misc_test test;

  exam::test_suite::test_case_type tc[3];

  t.add( &misc_test::type_traits_is_empty, test, "misc_test::type_traits_is_empty",
    t.add( &misc_test::type_traits_internals, test, "misc_test::type_traits_internals" ) );

  return t.girdle();
};

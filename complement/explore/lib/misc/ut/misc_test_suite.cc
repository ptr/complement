// -*- C++ -*- Time-stamp: <07/12/02 18:57:27 ptr>

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

  exam::test_suite::test_case_type tc[10];

  tc[0] = t.add( &misc_test::type_traits_internals, test, "traits_internals" );

  t.add( &misc_test::type_traits_is_void, test, "is_void", tc[0] );
  tc[1] = t.add( &misc_test::type_traits_is_integral, test, "is_integral", tc[0] );
  tc[2] = t.add( &misc_test::type_traits_is_floating_point, test, "is_floating_point", tc[0] );
  t.add( &misc_test::type_traits_is_array, test, "is_array", tc[0] );
  t.add( &misc_test::type_traits_is_pointer, test, "is_pointer", tc[0] );
  t.add( &misc_test::type_traits_is_lvalue_reference, test, "is_lvalue_reference", tc[0] );
  // t.add( &misc_test::type_traits_is_rvalue_reference, test, "is_rvalue_reference", tc[0] );

  t.add( &misc_test::type_traits_is_member_object_pointer, test, "is_member_object_pointer", tc[0] );
  t.add( &misc_test::type_traits_is_member_function_pointer, test, "is_member_function_pointer", tc[0] );

  t.add( &misc_test::type_traits_is_enum, test, "is_enum", tc[0] );
  t.add( &misc_test::type_traits_is_function, test, "is_function", tc[0] );

  // [20.4.4.2]
  t.add( &misc_test::type_traits_is_reference, test, "is_reference", tc[0] );
  tc[3] = t.add( &misc_test::type_traits_is_arithmetic, test, "is_arithmetic", tc + 1, tc + 3 );
  t.add( &misc_test::type_traits_is_fundamental, test, "is_fundamental", tc[3] );

  t.add( &misc_test::type_traits_is_object, test, "is_object", tc[0] );
  t.add( &misc_test::type_traits_is_scalar, test, "is_scalar", tc[0] );
  t.add( &misc_test::type_traits_is_compound, test, "is_compound", tc[0] );
  t.add( &misc_test::type_traits_is_member_pointer, test, "is_member_pointer", tc[0] );

  // [20.4.4.3]
  t.add( &misc_test::type_traits_is_const, test, "is_const", tc[0] );
  t.add( &misc_test::type_traits_is_volatile, test, "is_volatile", tc[0] );
  // t.add( &misc_test::type_traits_is_trivial, test, "is_trivial", tc[0] );
  // t.add( &misc_test::type_traits_is_standard_layout, test, "is_standard_layout", tc[0] );
  t.add( &misc_test::type_traits_is_pod, test, "is_pod", tc[0] );
  t.add( &misc_test::type_traits_is_empty, test, "is_empty", tc[0] );

  return t.girdle();
};

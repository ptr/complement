// -*- C++ -*- Time-stamp: <2011-04-30 23:04:54 ptr>

/*
 * Copyright (c) 2007, 2008
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#include "misc_test.h"

#include <config/feature.h>

void misc_test_suite_init( exam::test_suite& t, misc_test& test )
{
  exam::test_suite::test_case_type tc[10];

  tc[0] = t.add( &misc_test::type_traits_internals, test, "traits_internals" );

  t.add( &misc_test::type_traits_is_void, test, "is_void", tc[0] );
  tc[1] = t.add( &misc_test::type_traits_is_integral, test, "is_integral", tc[0] );
  tc[2] = t.add( &misc_test::type_traits_is_floating_point, test, "is_floating_point", tc[0] );
  t.add( &misc_test::type_traits_is_array, test, "is_array", tc[0] );
  t.add( &misc_test::type_traits_is_pointer, test, "is_pointer", tc[0] );
  t.add( &misc_test::type_traits_is_lvalue_reference, test, "is_lvalue_reference", tc[0] );
  t.add( &misc_test::type_traits_is_rvalue_reference, test, "is_rvalue_reference", tc[0] );

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
  tc[4] = t.add( &misc_test::type_traits_is_pod, test, "is_pod", tc[0] );
  t.add( &misc_test::type_traits_is_pod_compiler_supp, test, "is_pod_compiler_supp", tc[4] );
  t.add( &misc_test::type_traits_is_empty, test, "is_empty", tc[0] );

  t.add( &misc_test::ratio, test, "ratio", tc[0] );
}

void chrono_test_suite_init( exam::test_suite& t, chrono_test& chr )
{
  exam::test_suite::test_case_type tc[10];

  tc[0] = t.add( &chrono_test::duration_ctor, chr, "duration ctors" );
  tc[1] = t.add( &chrono_test::duration_arithmetic, chr, "duration arithmetic", tc[0] );
  tc[2] = t.add( &chrono_test::system_clock, chr, "system_clock", tc[1] );
}

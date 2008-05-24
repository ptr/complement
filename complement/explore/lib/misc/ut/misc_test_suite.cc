// -*- C++ -*- Time-stamp: <08/05/21 12:30:07 yeti>

/*
 * Copyright (c) 2007, 2008
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#include "misc_test_suite.h"
#include "misc_test.h"
#include "opts_test.h"

#include <config/feature.h>

int EXAM_IMPL(misc_test_suite)
{
  exam::test_suite t( "libmisc type_traits test" );
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
}

int EXAM_IMPL(options_test_suite)
{
  // test for options parsing
  exam::test_suite t( "libmisc, options test" );

  opts_test opts;

  t.add( &opts_test::bool_option_long, opts, "simple boolean option, long" , 
         t.add( &opts_test::bool_option, opts, "simple boolean option" ) );
  
  t.add( &opts_test::int_option_long, opts, "option with int parameter, long",
         t.add( &opts_test::int_option, opts, "option with int parameter" ) );

  t.add( &opts_test::add_check_flag , opts , "add_check_flag");
  t.add( &opts_test::add_get_opt , opts , "add_get_opts"); 
  t.add( &opts_test::option_position,opts,"option position");

  t.add( &opts_test::defaults, opts, "defaults" );
 
  t.add( &opts_test::bad_option, opts, "bad option" );
  t.add( &opts_test::bad_argument, opts, "bad argument" );
  t.add( &opts_test::unexpected_argument, opts, "unexpected_argument" );
  t.add( &opts_test::missing_argument, opts, "missing argument" );


  t.add( &opts_test::user_defined, opts, "user-defined type" );

  t.add( &opts_test::compound, opts, "compound" );

  t.add( &opts_test::multiple, opts,"multiple"); 

  t.add( &opts_test::multiple_compound, opts,"multiple_compound");
  
  t.add( &opts_test::args, opts,"args"); 

  t.add( &opts_test::stop, opts,"stop"); 
  
  // check whether autocomplement works
  t.add( &opts_test::autocomplement, opts,"autocomplement"); 
  t.add( &opts_test::autocomplement_failure, opts,"autocomplement_failure");

  t.add( &opts_test::multiple_args, opts,"multiple_args");

  t.add( &opts_test::help, opts, "help");
  
  return t.girdle();
}

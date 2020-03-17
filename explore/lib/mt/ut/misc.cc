// -*- C++ -*-

/*
 * Copyright (c) 2009
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#include "misc.h"

#include <config/feature.h>
#include <mt/callstack.h>
#include <typeinfo>

#include <iostream>

using namespace std;

int EXAM_IMPL(mt_test_misc::demangle)
{
#ifdef __FIT_CPP_DEMANGLE
  EXAM_CHECK( xmt::demangle( typeid(*this).name() ) == "mt_test_misc" );
  EXAM_CHECK( xmt::demangle( typeid(this).name() ) == "mt_test_misc*" );
  EXAM_CHECK( string( typeid(&mt_test_misc::demangle).name() ) != "int (mt_test_misc::*)(exam::test_suite*, int)" );
  EXAM_CHECK( xmt::demangle( typeid(&mt_test_misc::demangle).name() ) == "int (mt_test_misc::*)(exam::test_suite*, int)" );
#else
  throw exam::skip_exception();
#endif

  return EXAM_RESULT;
}

int EXAM_IMPL(mt_test_misc::function)
{
#ifdef __GNUC__
  EXAM_CHECK( string( "int mt_test_misc::function(exam::test_suite*, int)" ) == __PRETTY_FUNCTION__ );
  EXAM_CHECK( string( "function" ) == __func__ );
#else
  throw exam::skip_exception();
#endif

  return EXAM_RESULT;
}

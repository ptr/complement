// -*- C++ -*- Time-stamp: <07/08/06 10:26:25 ptr>

/*
 * Copyright (c) 2007
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#include "misc_test.h"

#include <misc/type_traits.h>
#include <iostream>

using namespace std;

class MyType
{
  public:

    virtual int abstract() = 0;

  private:
    int a;
    int n;
};

class MyTypeOther
{
  public:

  private:
    int a;
    int n;
};

int f()
{
  return 0;
}

template <class T>
static bool q( T v )
{
  return std::tr1::detail::__instance<T>::__value;
}

int EXAM_IMPL(misc_test::type_traits)
{
  EXAM_CHECK( std::tr1::detail::__instance<int []>::__value == true );
  EXAM_CHECK( std::tr1::detail::__instance<int *>::__value == true );
  EXAM_CHECK( std::tr1::detail::__instance<int&>::__value == false );
  EXAM_CHECK( std::tr1::detail::__instance<MyTypeOther>::__value == true );
  EXAM_CHECK( std::tr1::detail::__instance<int>::__value == true );
  EXAM_CHECK( std::tr1::detail::__instance<MyType>::__value == false );
  EXAM_CHECK( std::tr1::detail::__instance<int (*)()>::__value == true );
  EXAM_CHECK( std::tr1::detail::__instance<int (&)()>::__value == false );
  EXAM_CHECK( q(f) == true );

  return EXAM_RESULT;
}

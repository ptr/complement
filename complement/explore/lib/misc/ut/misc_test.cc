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
bool q( T v )
{
  return std::tr1::detail::__instance<T>::__value;
}

int EXAM_IMPL(misc_test::type_traits_internals)
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

class empty
{
};

class not_empty1
{
  private:
    int k;
};

class not_empty2
{
  private:
    int f() const
      { return 0; }
};

class not_empty3
{
  private:
    virtual int f() const
      { return 0; }
};

class not_empty4 :
    public not_empty2
{
};

class not_empty5 :
    public not_empty3
{
};

int EXAM_IMPL(misc_test::type_traits_is_empty)
{
  EXAM_CHECK( std::tr1::is_empty<empty>::value == true );
  EXAM_CHECK( std::tr1::is_empty<not_empty1>::value == false );
  // EXAM_CHECK( std::tr1::is_empty<not_empty2>::value == false );
  EXAM_CHECK( std::tr1::is_empty<not_empty3>::value == false );
  EXAM_CHECK( std::tr1::is_empty<int>::value == false );
  // EXAM_CHECK( std::tr1::is_empty<int (&)()>::value == false );
  // EXAM_CHECK( std::tr1::is_empty<not_empty4>::value == false );
  EXAM_CHECK( std::tr1::is_empty<not_empty5>::value == false );

  return EXAM_RESULT;
}


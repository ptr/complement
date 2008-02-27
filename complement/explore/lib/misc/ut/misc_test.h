// -*- C++ -*- Time-stamp: <07/12/02 18:50:20 ptr>

/*
 * Copyright (c) 2007
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#ifndef __MISC_TEST_H
#define __MISC_TEST_H

#define FIT_EXAM

#include <exam/suite.h>

class misc_test
{
  public:
    // implementation
    int EXAM_DECL(type_traits_internals);
    // [20.4.4.1]
    int EXAM_DECL(type_traits_is_void);
    int EXAM_DECL(type_traits_is_integral);
    int EXAM_DECL(type_traits_is_floating_point);
    int EXAM_DECL(type_traits_is_array);
    int EXAM_DECL(type_traits_is_pointer);
    int EXAM_DECL(type_traits_is_lvalue_reference);
    // int EXAM_DECL(type_traits_is_rvalue_reference);
    int EXAM_DECL(type_traits_is_member_object_pointer);
    int EXAM_DECL(type_traits_is_member_function_pointer);
    int EXAM_DECL(type_traits_is_enum);
    int EXAM_DECL(type_traits_is_union);
    int EXAM_DECL(type_traits_is_class);
    int EXAM_DECL(type_traits_is_function);

    // [20.4.4.2]
    int EXAM_DECL(type_traits_is_reference);
    int EXAM_DECL(type_traits_is_arithmetic);
    int EXAM_DECL(type_traits_is_fundamental);
    int EXAM_DECL(type_traits_is_object);
    int EXAM_DECL(type_traits_is_scalar);
    int EXAM_DECL(type_traits_is_compound);
    int EXAM_DECL(type_traits_is_member_pointer);

    // [20.4.4.3]
    int EXAM_DECL(type_traits_is_const);
    int EXAM_DECL(type_traits_is_volatile);
    int EXAM_DECL(type_traits_is_trivial);
    int EXAM_DECL(type_traits_is_standard_layout);
    int EXAM_DECL(type_traits_is_pod);
    int EXAM_DECL(type_traits_is_empty);
};

#endif // __MISC_TEST_H

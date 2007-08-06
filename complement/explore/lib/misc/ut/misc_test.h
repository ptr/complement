// -*- C++ -*- Time-stamp: <07/07/16 21:01:43 ptr>

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
    int EXAM_DECL(type_traits_internals);
    int EXAM_DECL(type_traits_is_empty);
};

#endif // __MISC_TEST_H

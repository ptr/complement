// -*- C++ -*- Time-stamp: <08/05/01 15:17:31 ptr>

/*
 * Copyright (c) 2008
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#ifndef __OPTS_TEST_H
#define __OPTS_TEST_H

#define FIT_EXAM

#include <exam/suite.h>

class opts_test
{
  public:
    // implementation
    int EXAM_DECL(bool_option);
    int EXAM_DECL(bool_option_long);
    int EXAM_DECL(int_option);
    int EXAM_DECL(int_option_long);
    int EXAM_DECL(bad_option);
    int EXAM_DECL(bad_argument);
    int EXAM_DECL(multiple);
    int EXAM_DECL(compound);
    int EXAM_DECL(args);
    int EXAM_DECL(stop);
    int EXAM_DECL(user_defined);
    int EXAM_DECL(reduction);
};

#endif // __MISC_TEST_H

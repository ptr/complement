// -*- C++ -*- Time-stamp: <08/08/14 15:44:08 yeti>

/*
 * Copyright (c) 2008
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 */

#ifndef __ALLOC_PERF_SUITE_H
#define __ALLOC_PERF_SUITE_H

#include <exam/suite.h>

class sleepycat_test
{
  public:
    int EXAM_DECL(hash_open_cxx);
    int EXAM_DECL(hash_open);
};

#endif // __ALLOC_PERF_SUITE_H

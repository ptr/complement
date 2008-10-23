// -*- C++ -*- Time-stamp: <08/08/13 13:26:19 yeti>

/*
 * Copyright (c) 2008
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 */

#ifndef __ALLOC_PERF_SUITE_H
#define __ALLOC_PERF_SUITE_H

#include <exam/suite.h>

class alloc_test
{
  public:
    int EXAM_DECL(alloc);
    int EXAM_DECL(alloc5000);
    int EXAM_DECL(alloc_mix);
    int EXAM_DECL(alloc_t5);
};

#endif // __ALLOC_PERF_SUITE_H

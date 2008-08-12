// -*- C++ -*- Time-stamp: <08/08/12 18:28:37 yeti>

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
};

#endif // __ALLOC_PERF_SUITE_H

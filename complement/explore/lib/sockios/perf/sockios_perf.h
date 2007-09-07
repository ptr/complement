// -*- C++ -*- Time-stamp: <07/09/06 11:12:29 ptr>

/*
 *
 * Copyright (c) 2007
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __sockios_perf_h
#define __sockios_perf_h

#include <exam/suite.h>
#include <string>
#include <mt/shm.h>

class sockios_perf
{
  public:
    sockios_perf();
    ~sockios_perf();

    int EXAM_DECL(exchange1);
    int EXAM_DECL(exchange2);
    int EXAM_DECL(exchange3);

  private:
    // const std::string fname;
    // xmt::shm_alloc<0> seg;
    // xmt::allocator_shm<xmt::__condition<true>,0> shm_cnd;
    // xmt::allocator_shm<xmt::__barrier<true>,0> shm_b;
};

#endif // __sockios_perf_h

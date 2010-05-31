// -*- C++ -*- Time-stamp: <10/05/31 16:55:39 ptr>

/*
 *
 * Copyright (c) 2010
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include "mt_perf.h"
#include <exam/suite.h>

#include <mt/uid.h>

using namespace std;
using namespace std::tr2;

mt_perf::mt_perf()
{
}

mt_perf::~mt_perf()
{
}

int EXAM_IMPL(mt_perf::uuid)
{
  xmt::uuid_type u1;

  for ( int i = 0; i < 10000; ++i ) {
    u1 = xmt::uid();
  }

  return EXAM_RESULT;
}

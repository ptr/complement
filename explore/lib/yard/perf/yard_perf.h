// -*- C++ -*- Time-stamp: <10/04/01 12:09:31 ptr>

/*
 *
 * Copyright (c) 2010
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __yard_perf_h
#define __yard_perf_h

#include <exam/suite.h>
#include <string>

#include <mt/shm.h>
#include <sys/wait.h>
#include <signal.h>
#include <algorithm>

#include <mt/mutex>
#include <mt/condition_variable>
#include <mt/date_time>

class yard_perf
{
  public:
    yard_perf();
    ~yard_perf();

    // int EXAM_DECL(local);
};

#endif // __yard_perf_h

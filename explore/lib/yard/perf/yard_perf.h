// -*- C++ -*- Time-stamp: <10/05/13 11:30:04 ptr>

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

    int EXAM_DECL(put);
    int EXAM_DECL(put_more);
    int EXAM_DECL(put_more_more);
    int EXAM_DECL(put_get);
    int EXAM_DECL(put_object);
    int EXAM_DECL(put_object_r2);
    int EXAM_DECL(put_mess);
};

#endif // __yard_perf_h

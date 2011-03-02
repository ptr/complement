// -*- C++ -*- Time-stamp: <2011-03-02 18:35:43 ptr>

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

    int EXAM_DECL(packing);
    int EXAM_DECL(unpacking);

    int EXAM_DECL(consecutive_insert);
    int EXAM_DECL(consecutive_insert_big);
    int EXAM_DECL(random_insert_big);
    int EXAM_DECL(consecutive_insert_with_data);
    int EXAM_DECL(random_insert_with_data);
    int EXAM_DECL(multiple_files);
    int EXAM_DECL(random_lookup);

    int EXAM_DECL(mess);
    int EXAM_DECL(put_revisions);
    int EXAM_DECL(mess_insert);
};

#endif // __yard_perf_h

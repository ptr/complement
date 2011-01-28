// -*- C++ -*- Time-stamp: <2011-01-28 18:09:31 ptr>

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

    int EXAM_DECL(put);
    int EXAM_DECL(put_more);
    int EXAM_DECL(put_more_more);
    int EXAM_DECL(put_get);
    int EXAM_DECL(put_object);
    int EXAM_DECL(put_object_r2);

    int EXAM_DECL(mess);
    int EXAM_DECL(put_revisions);
};

#endif // __yard_perf_h

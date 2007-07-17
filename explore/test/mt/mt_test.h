// -*- C++ -*- Time-stamp: <07/07/16 21:01:43 ptr>

/*
 * Copyright (c) 2006, 2007
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#ifndef __MT_TEST_H
#define __MT_TEST_H

#define FIT_EXAM

#include <exam/suite.h>

class mt_test
{
  public:
    int EXAM_DECL(barrier);
    int EXAM_DECL(join_test);
    int EXAM_DECL(barrier2);
    int EXAM_DECL(yield);
    int EXAM_DECL(mutex_test);
    int EXAM_DECL(spinlock_test);
    int EXAM_DECL(recursive_mutex_test);

    int EXAM_DECL(fork);
    int EXAM_DECL(pid);
    int EXAM_DECL(shm_segment);
    int EXAM_DECL(shm_alloc);
    int EXAM_DECL(fork_shm);
    int EXAM_DECL(shm_named_obj);
    int EXAM_DECL(thr_mgr);

    int EXAM_DECL(shm_init);
    int EXAM_DECL(shm_finit);

    int EXAM_DECL(shm_named_obj_more);

  private:
    // static xmt::Thread::ret_code thread_entry_call( void * );
    // static int x;
};

#endif // __MT_TEST_H

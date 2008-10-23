// -*- C++ -*- Time-stamp: <08/03/26 10:08:36 ptr>

/*
 * Copyright (c) 2006-2008
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#ifndef __MT_TEST_H
#define __MT_TEST_H

#define FIT_EXAM

#include <exam/suite.h>
#include <mt/shm.h>

class mt_test
{
  public:
    int EXAM_DECL(callstack);
    int EXAM_DECL(barrier);
    int EXAM_DECL(join_test);
    int EXAM_DECL(barrier2);
    int EXAM_DECL(yield);
    int EXAM_DECL(mutex_test);
    int EXAM_DECL(spinlock_test);
    int EXAM_DECL(recursive_mutex_test);

    int EXAM_DECL(fork);
    int EXAM_DECL(pid);
    int EXAM_DECL(thr_mgr);

  private:
    // static xmt::Thread::ret_t thread_entry_call( void * );
    // static int x;
};

#endif // __MT_TEST_H

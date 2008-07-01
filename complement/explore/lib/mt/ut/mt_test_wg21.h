// -*- C++ -*- Time-stamp: <08/02/25 12:12:20 ptr>

/*
 * Copyright (c) 2006-2008
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#ifndef __MT_TEST_WG21_H
#define __MT_TEST_WG21_H

#define FIT_EXAM

#include <exam/suite.h>
// #include <mt/shm.h>

class mt_test_wg21
{
  public:
    int EXAM_DECL(date_time);
    int EXAM_DECL(thread_call);
    int EXAM_DECL(mutex_test);
    int EXAM_DECL(barrier);
    int EXAM_DECL(semaphore);
    int EXAM_DECL(fork);
    int EXAM_DECL(uid);

  private:
    // static xmt::Thread::ret_t thread_entry_call( void * );
    // static int x;
};

#endif // __MT_TEST_WG21_H

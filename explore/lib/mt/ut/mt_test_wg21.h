// -*- C++ -*- Time-stamp: <10/07/01 15:39:23 ptr>

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

class mt_test_wg21
{
  public:
    int EXAM_DECL(date_time);
    int EXAM_DECL(thread_call);
    int EXAM_DECL(mutex_test);
    int EXAM_DECL(mutex_rw_test);
    int EXAM_DECL(barrier);
    int EXAM_DECL(semaphore);
    int EXAM_DECL(fork);
    int EXAM_DECL(condition_var);

  private:
    // static xmt::Thread::ret_t thread_entry_call( void * );
    // static int x;
};

class uid_test_wg21
{
  public:
    int EXAM_DECL(uidstr);
    int EXAM_DECL(uid);
    int EXAM_DECL(hostidstr);
    int EXAM_DECL(hostid);
    int EXAM_DECL(uidconv);
    int EXAM_DECL(uid_stream);
};

#endif // __MT_TEST_WG21_H

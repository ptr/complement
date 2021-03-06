// -*- C++ -*-

/*
 * Copyright (c) 2006-2008, 2017
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
    int EXAM_DECL(pid);

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
    int EXAM_DECL(version);
    int EXAM_DECL(istream);
    int EXAM_DECL(istream_iterator);
    int EXAM_DECL(istream_iterator_ctor);
    int EXAM_DECL(copy_n);
    int EXAM_DECL(sentry);
};

#endif // __MT_TEST_WG21_H

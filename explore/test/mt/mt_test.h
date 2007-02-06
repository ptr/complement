// -*- C++ -*- Time-stamp: <07/02/06 09:48:27 ptr>

/*
 * Copyright (c) 2006, 2007
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#ifndef __MT_TEST_H
#define __MT_TEST_H

class mt_test
{
  public:
    void barrier();
    void join_test();
    void barrier2();
    void yield();
    void mutex_test();
    void spinlock_test();
    void recursive_mutex_test();

    void fork();
    void pid();
    void shm_segment();
    void shm_alloc();
    void fork_shm();
    void shm_named_obj();
    void thr_mgr();

  private:
    // static xmt::Thread::ret_code thread_entry_call( void * );
    // static int x;
};

#endif // __MT_TEST_H

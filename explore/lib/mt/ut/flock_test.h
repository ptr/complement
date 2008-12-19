// -*- C++ -*- Time-stamp: <08/03/26 10:08:36 ptr>

/*
 * Copyright (c) 2004, 2006-2008
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#ifndef __FLOCK_TEST_H
#define __FLOCK_TEST_H

#define FIT_EXAM

#include <exam/suite.h>
// #include <mt/shm.h>
#include <string>

class flock_test
{
  public:
    ~flock_test();

    int EXAM_DECL(create);
    int EXAM_DECL(read_lock);

  private:
    // static xmt::Thread::ret_t thread_entry_call( void * );
    // static int x;
    std::string fname;
};

#endif // __FLOCK_TEST_H


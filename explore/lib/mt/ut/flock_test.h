// -*- C++ -*- Time-stamp: <08/12/23 23:04:14 ptr>

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
    int EXAM_DECL(write_lock);
    int EXAM_DECL(format);
    // int EXAM_DECL(write_profane);
    int EXAM_DECL(wr_lock);
    int EXAM_DECL(rw_lock);

  private:
    std::string fname;
};

#endif // __FLOCK_TEST_H


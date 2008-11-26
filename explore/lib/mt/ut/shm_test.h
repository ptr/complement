// -*- C++ -*- Time-stamp: <08/11/27 00:00:36 ptr>

/*
 * Copyright (c) 2006, 2007
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#ifndef __SHM_TEST_H
#define __SHM_TEST_H

#define FIT_EXAM

#include <exam/suite.h>
#include <mt/shm.h>

class shm_test
{
  public:
    shm_test();
    ~shm_test();

    int EXAM_DECL(shm_segment);
    int EXAM_DECL(shm_alloc);
    int EXAM_DECL(fork_shm);
    int EXAM_DECL(shm_named_obj);

    int EXAM_DECL(shm_named_obj_more);

  private:
    xmt::shm_alloc<1> seg1;
    static const char fname1[];

    int seg_id( int );
};

#endif // __SHM_TEST_H

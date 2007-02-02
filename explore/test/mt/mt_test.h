// -*- C++ -*- Time-stamp: <07/02/02 16:45:13 ptr>

/*
 * Copyright (c) 2006
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#ifndef __MT_TEST_H
#define __MT_TEST_H

struct mt_test
{
    void fork();
    void pid();
    void shm_segment();
    void shm_alloc();
    void fork_shm();
    void shm_named_obj();
    void thr_mgr();
};

#endif // __MT_TEST_H

// -*- C++ -*- Time-stamp: <08/03/25 06:22:29 ptr>

/*
 *
 * Copyright (c) 2002, 2003, 2005-2008
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __sockios2_test_h
#define __sockios2_test_h

#include <exam/suite.h>
// #include <mt/shm.h>

class sockios2_test
{
  public:
    sockios2_test();
    ~sockios2_test();

    int EXAM_DECL(srv_core);
    int EXAM_DECL(connect_disconnect);
    int EXAM_DECL(processor_core);
    int EXAM_DECL(fork);
};

#endif // __sockios2_test_h

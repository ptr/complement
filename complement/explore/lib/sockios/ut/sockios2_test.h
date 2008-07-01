// -*- C++ -*- Time-stamp: <08/07/01 12:21:46 yeti>

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

class sockios2_test
{
  public:
    sockios2_test();
    ~sockios2_test();

    int EXAM_DECL(srv_core);
    int EXAM_DECL(connect_disconnect);
    int EXAM_DECL(disconnect);
    int EXAM_DECL(processor_core);
    int EXAM_DECL(fork);
    int EXAM_DECL(srv_sigpipe);
    int EXAM_DECL(read0);
};

#endif // __sockios2_test_h

// -*- C++ -*- Time-stamp: <07/10/19 18:38:53 yeti>

/*
 *
 * Copyright (c) 2007
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __intercessor_test_h
#define __intercessor_test_h

#include <exam/suite.h>

class intercessor_test
{
  public:

    int EXAM_DECL(base);
    int EXAM_DECL(processor);
    int EXAM_DECL(processor_post);
    int EXAM_DECL(processor_external_post);
    int EXAM_DECL(negative);

  private:
};

#endif // __intercessor_test_h

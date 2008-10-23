// -*- C++ -*- Time-stamp: <07/10/19 18:38:53 yeti>

/*
 *
 * Copyright (c) 2007
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __http_test_h
#define __http_test_h

#include <exam/suite.h>

class http_test
{
  public:

    int EXAM_DECL(header_io);
    int EXAM_DECL(header_sp);
    int EXAM_DECL(command);
    int EXAM_DECL(base_response);
    int EXAM_DECL(request);
    int EXAM_DECL(response);

  private:
};

#endif // __http_test_h

// -*- C++ -*- Time-stamp: <07/07/16 21:38:13 ptr>

/*
 * Copyright (c) 2004, 2007
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#include <mt/xmt.h>

#include <exam/suite.h>

int EXAM_IMPL(timespec_diff)
{
  timespec t1;
  t1.tv_sec = 1083852875;
  t1.tv_nsec = 16629000;

  timespec t2;
  t2.tv_sec = 1083852871;
  t2.tv_nsec = 365378000;

  timespec t3 = t1 - t2;

  EXAM_CHECK( t3.tv_sec == 3 );
  EXAM_CHECK( t3.tv_nsec == 651251000 );

  t3 = t1;
  t3 -= t2;

  EXAM_CHECK( t3.tv_sec == 3 );
  EXAM_CHECK( t3.tv_nsec == 651251000 );

  t1.tv_sec = 1;
  t1.tv_nsec = 1;

  t2.tv_sec = 0;
  t2.tv_nsec = 1;

  t3 = t1 - t2;

  EXAM_CHECK( t3.tv_sec == 1 );
  EXAM_CHECK( t3.tv_nsec == 0 );

  t3 = t1;
  t3 -= t2;
  
  EXAM_CHECK( t3.tv_sec == 1 );
  EXAM_CHECK( t3.tv_nsec == 0 );

  return EXAM_RESULT;
}

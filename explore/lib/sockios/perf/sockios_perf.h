// -*- C++ -*- Time-stamp: <07/09/12 22:57:45 ptr>

/*
 *
 * Copyright (c) 2007
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __sockios_perf_h
#define __sockios_perf_h

#include <exam/suite.h>
#include <string>
#include <mt/shm.h>

class sockios_perf_SrvR
{
  public:
    sockios_perf_SrvR();
    ~sockios_perf_SrvR();

    int EXAM_DECL(r1);
    int EXAM_DECL(r2);
    int EXAM_DECL(r3);
    int EXAM_DECL(r4);
    int EXAM_DECL(r5);
    int EXAM_DECL(r6);
    int EXAM_DECL(r7);
    int EXAM_DECL(r8);
    int EXAM_DECL(r9);
    int EXAM_DECL(r10);
    int EXAM_DECL(r11);
};

class sockios_perf_SrvW
{
  public:
    sockios_perf_SrvW();
    ~sockios_perf_SrvW();

    int EXAM_DECL(r1);
    int EXAM_DECL(r2);
    int EXAM_DECL(r3);
    int EXAM_DECL(r4);
    int EXAM_DECL(r5);
    int EXAM_DECL(r6);
    int EXAM_DECL(r7);
    int EXAM_DECL(r8);
    int EXAM_DECL(r9);
    int EXAM_DECL(r10);
    int EXAM_DECL(r11);
};

class sockios_perf_SrvRW
{
  public:
    sockios_perf_SrvRW();
    ~sockios_perf_SrvRW();

    int EXAM_DECL(r1);
    int EXAM_DECL(r2);
    int EXAM_DECL(r3);
    int EXAM_DECL(r4);
    int EXAM_DECL(r5);
    int EXAM_DECL(r6);
    int EXAM_DECL(r7);
    int EXAM_DECL(r8);
    int EXAM_DECL(r9);
    int EXAM_DECL(r10);
    int EXAM_DECL(r11);
};

#endif // __sockios_perf_h

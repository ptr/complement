// -*- C++ -*- Time-stamp: <2010-11-10 13:59:56 ptr>

/*
 *
 * Copyright (c) 2010
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __syslog_test_h
#define __syslog_test_h

#include <exam/suite.h>

struct syslog_test
{
    int EXAM_DECL(core_test);
    int EXAM_DECL(level_facility_conversions);
};

#endif // __syslog_test_h

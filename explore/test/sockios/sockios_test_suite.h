// -*- C++ -*- Time-stamp: <06/12/18 16:52:16 ptr>

/*
 *
 * Copyright (c) 2002, 2003, 2005, 2006
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __sockios_test_suite_h
#define __sockios_test_suite_h

#include <boost/test/unit_test.hpp>

struct sockios_test_suite :
    public boost::unit_test_framework::test_suite
{
    sockios_test_suite();
};

#endif // __sockios_test_suite_h

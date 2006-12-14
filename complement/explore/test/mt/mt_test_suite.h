// -*- C++ -*- Time-stamp: <06/12/14 10:42:18 ptr>

/*
 * Copyright (c) 2006
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#ifndef __MT_TEST_SUITE_H
#define __MT_TEST_SUITE_H

#include <boost/test/unit_test.hpp>

struct mt_test_suite :
    public boost::unit_test_framework::test_suite
{
    mt_test_suite();
};

#endif // __MT_TEST_SUITE_H

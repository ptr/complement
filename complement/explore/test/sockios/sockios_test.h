// -*- C++ -*- Time-stamp: <06/12/18 16:54:00 ptr>

/*
 *
 * Copyright (c) 2002, 2003, 2005, 2006
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __sockios_test_h
#define __sockios_test_h

struct sockios_test
{
    void hostname_test();
    void service_test();

    void hostaddr_test1();
    void hostaddr_test2();
    void hostaddr_test3();

    void ctor_dtor();

    void sigpipe();
};

#endif // __sockios_test_h

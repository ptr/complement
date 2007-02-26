// -*- C++ -*- Time-stamp: <07/02/26 15:09:26 ptr>

/*
 *
 * Copyright (c) 2002, 2003, 2005, 2006, 2007
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __sockios_test_h
#define __sockios_test_h

struct names_sockios_test
{
    void hostname_test();
    void service_test();

    void hostaddr_test1();
    void hostaddr_test2();
    void hostaddr_test3();
};

struct sockios_test
{
    sockios_test();
    ~sockios_test();

    void init();
    void finit();

    void ctor_dtor();

    void long_msg();

    void sigpipe();
    void read0();
    void read0_srv();
    void long_block_read();

    void srv2_fork();
};

#endif // __sockios_test_h

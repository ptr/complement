// -*- C++ -*- Time-stamp: <08/06/09 20:27:19 yeti>

/*
 *
 * Copyright (c) 2002, 2003, 2005-2007
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __sockios_test_h
#define __sockios_test_h

#include <exam/suite.h>
#include <mt/shm.h>

struct names_sockios_test
{
    int EXAM_DECL(hostname_test);
    int EXAM_DECL(service_test);

    int EXAM_DECL(hostaddr_test1);
    int EXAM_DECL(hostaddr_test2);
    int EXAM_DECL(hostaddr_test3);
};

#if 0
class sockios_test
{
  public:
    sockios_test();
    ~sockios_test();

    int EXAM_DECL(ctor_dtor);

    int EXAM_DECL(long_msg);

    int EXAM_DECL(sigpipe);
    int EXAM_DECL(read0);
    int EXAM_DECL(read0_srv);
    int EXAM_DECL(long_block_read);

    int EXAM_DECL(srv2_fork);

  private:
    const std::string fname;
    xmt::shm_alloc<0> seg;
    xmt::allocator_shm<xmt::__condition<true>,0> shm_cnd;
    xmt::allocator_shm<xmt::__barrier<true>,0> shm_b;
};
#endif

#endif // __sockios_test_h

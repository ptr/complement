// -*- C++ -*- Time-stamp: <09/10/06 15:06:18 ptr>

/*
 *
 * Copyright (c) 2002, 2003, 2005-2009
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __sockios_test_h
#define __sockios_test_h

#include <exam/suite.h>

class sockios_test
{
  public:
    sockios_test();
    ~sockios_test();

    int EXAM_DECL(srv_core);
    int EXAM_DECL(connect_disconnect);
    int EXAM_DECL(disconnect);
    int EXAM_DECL(disconnect_rawclnt);
    int EXAM_DECL(processor_core_one_local);
    int EXAM_DECL(processor_core_two_local);
    int EXAM_DECL(processor_core_getline);
    int EXAM_DECL(processor_core_income_data);
    int EXAM_DECL(fork);
    int EXAM_DECL(income_data);
    int EXAM_DECL(check_rdtimeout_fail);
    int EXAM_DECL(check_rdtimeout);
    int EXAM_DECL(open_timeout);
    int EXAM_DECL(srv_sigpipe);
    int EXAM_DECL(read0);

    int EXAM_DECL(few_packets);
    int EXAM_DECL(few_packets_loop);

    int EXAM_DECL(two_ports);

    int EXAM_DECL(service_stop);
    int EXAM_DECL(quants_reader);
    int EXAM_DECL(echo);
    int EXAM_DECL(ugly_echo);
};

#endif // __sockios2_test_h

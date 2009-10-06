// -*- C++ -*- Time-stamp: <09/10/06 15:38:25 ptr>

/*
 *
 * Copyright (c) 2009
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __unix_socket_h
#define __unix_socket_h

#include <exam/suite.h>

struct unix_sockios_test
{
    int EXAM_DECL(core_test);
    int EXAM_DECL(core_write_test);
    int EXAM_DECL(stream_core_test);
    int EXAM_DECL(stream_core_write_test);
    int EXAM_DECL(processor_core_one_local);
    int EXAM_DECL(processor_core_two_local);
    int EXAM_DECL(processor_core_getline);
    int EXAM_DECL(processor_core_income_data);
    int EXAM_DECL(income_data);
    int EXAM_DECL(open_timeout);
};

#endif // __unix_socket_h

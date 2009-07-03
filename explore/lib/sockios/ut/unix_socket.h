// -*- C++ -*- Time-stamp: <09/07/03 14:51:31 ptr>

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
};

#endif // __unix_socket_h

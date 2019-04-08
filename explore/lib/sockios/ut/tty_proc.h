/* -*- C++ -*- */

/*
 *
 * Copyright (c) 2019
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __tty_proc_h
#define __tty_proc_h

#include <exam/suite.h>

struct tty_processor_test
{
    int EXAM_DECL(tty_sockbuf);
    int EXAM_DECL(tty_sockstream);
    int EXAM_DECL(tty_processor);
};

#endif // __tty_proc_h

/*
 *
 * Copyright (c) 2002, 2003, 2005-2007
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __names_h
#define __names_h

#include <exam/suite.h>

struct names_sockios_test
{
    int EXAM_DECL(hostname_test);
    int EXAM_DECL(service_test);

    int EXAM_DECL(hostaddr_test1);
    int EXAM_DECL(hostaddr_test2);
    int EXAM_DECL(hostaddr_test3);
};

#endif // __names_h

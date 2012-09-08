// -*- C++ -*- Time-stamp: <2012-02-08 22:16:30 ptr>

/*
 * Copyright (c) 2012
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __SOCKIOS_ACCESS_POINT_H
#define __SOCKIOS_ACCESS_POINT_H

#ifndef __config_feature_h
#include <config/feature.h>
#endif

#include <forward_list>
// #include <string>

// #include <netdb.h>
// #include <netinet/in.h>
#include <sys/socket.h>

#include <mt/uid.h>

struct access_point
{
    int type;
    // std::forward_list<std::string> domains;
    std::forward_list<sockaddr> addr;
    xmt::uuid_type hostid;
};

#endif // __SOCKIOS_ACCESS_POINT_H

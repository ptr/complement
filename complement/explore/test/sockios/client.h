// -*- C++ -*- Time-stamp: <05/12/20 09:52:15 ptr>

/*
 *
 * Copyright (c) 2002, 2003, 2005
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 2.1
 *
 * This material is provided "as is", with absolutely no warranty expressed
 * or implied. Any use is at your own risk.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.
 */

#ifndef __Client_h
#define __Client_h

// Clients simulator

class Client
{
  public:
    static void client1();
    static void client_nonlocal_ack();
    static void client_nonlocal_nac();
    static void client_local_ack();
    static void udp_client1();
    static void client_dup();
};

#endif // __Client_h

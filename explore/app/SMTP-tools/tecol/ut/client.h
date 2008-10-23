// -*- C++ -*- Time-stamp: <03/08/15 13:25:22 ptr>

/*
 *
 * Copyright (c) 2002, 2003
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 1.2
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

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "@(#)$Id: client.h,v 1.1 2003/10/06 08:01:06 ptr Exp $"
#  else
#ident "@(#)$Id: client.h,v 1.1 2003/10/06 08:01:06 ptr Exp $"
#  endif
#endif

// Clients simulator

class Client
{
  public:
    static void client1();
};

#endif // __Client_h

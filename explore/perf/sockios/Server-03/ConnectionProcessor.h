// -*- C++ -*- Time-stamp: <02/11/17 14:18:36 ptr>

/*
 *
 * Copyright (c) 2002
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 1.0
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

#ifndef __ConnectionProcessor_h
#define __ConnectionProcessor_h

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "@(#)$Id$"
#  else
#ident "@(#)$Id$"
#  endif
#endif

#include <sockios/sockstream>

class ConnectionProcessor
{
  public:
    ConnectionProcessor( std::sockstream& );

    void connect( std::sockstream& );
    void close();
};

// #define OUT_MSG(msg) pr_lock.lock(); cerr << msg << endl; pr_lock.unlock()
#define OUT_MSG(msg) cerr << msg << endl


#endif // __ConnectionProcessor_h

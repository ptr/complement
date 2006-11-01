// -*- C++ -*- Time-stamp: <06/07/08 00:13:11 ptr>

/*
 *
 * Copyright (c) 2002, 2005
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

#ifndef __ConnectionProcessor_h
#define __ConnectionProcessor_h

#include <sockios/sockstream>


/*
Class that provide processing of clints connections;
it MUST has default constructor, and method 'connect( std::sockstream& )'.
Method 'connect'is real processing, it parameter is stream, that associated
with socket, connected to client (that's true for tcp connections).
Instance of that class has full control over connection, and connection will
be closed after return from 'connect' method.
*/ 

class ConnectionProcessor // 
{
  public:
    ConnectionProcessor( std::sockstream& );

    void connect( std::sockstream& );
    void close();
};

class ConnectionProcessor2 // 
{
  public:
    ConnectionProcessor2( std::sockstream& );

    void connect( std::sockstream& );
    void close();

    int count;
};

#endif // __ConnectionProcessor_h

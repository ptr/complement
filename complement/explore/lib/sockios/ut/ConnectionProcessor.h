// -*- C++ -*- Time-stamp: <07/07/19 00:16:00 ptr>

/*
 *
 * Copyright (c) 2002, 2005, 2007
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
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

class trivial_sockios_test
{
  public:
    trivial_sockios_test();

    int EXAM_DECL(simple);
    int EXAM_DECL(simple_udp);
    int EXAM_DECL(listen_iface);
    int EXAM_DECL(shared_socket);

    int EXAM_DECL(srv_close_connection);
    int EXAM_DECL(client_close_socket);

  private:
    in_addr hostaddr;
    // sockaddr hostaddr;
    in_addr localaddr;
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

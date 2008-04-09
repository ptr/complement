// -*- C++ -*- Time-stamp: <04/05/25 14:26:05 ptr>

#ifndef __ConnectionProcessor_h
#define __ConnectionProcessor_h

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "@(#)$Id: ConnectionProcessor.h,v 1.1 2004/06/16 14:28:16 ptr Exp $"
#  else
#ident "@(#)$Id: ConnectionProcessor.h,v 1.1 2004/06/16 14:28:16 ptr Exp $"
#  endif
#endif

#include <sockios/sockstream>
#include <map>
#include <algorithm>

#include <mt/xmt.h>

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

  private:
    enum mode {
      command,
      data
    };

    mode state;

    static std::sockstream tecol;
};

#endif // __ConnectionProcessor_h

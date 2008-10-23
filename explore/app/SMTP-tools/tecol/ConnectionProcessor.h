// -*- C++ -*- Time-stamp: <04/05/07 13:07:52 ptr>

#ifndef __ConnectionProcessor_h
#define __ConnectionProcessor_h

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "@(#)$Id: ConnectionProcessor.h,v 1.3 2004/06/16 14:26:39 ptr Exp $"
#  else
#ident "@(#)$Id: ConnectionProcessor.h,v 1.3 2004/06/16 14:26:39 ptr Exp $"
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
    struct conn
    {
        conn()
          {
            s = 0;
            tm[0].tv_sec  = tm[1].tv_sec  = 0;
            tm[0].tv_nsec = tm[1].tv_nsec = 0;
          }
        conn( std::sockstream *_s ) :
            s( _s )
          {
            __impl::Thread::gettime( &tm[0] );
            tm[1].tv_sec = 0;
            tm[1].tv_nsec = 0;
          }
        conn( std::sockstream& _s ) :
            s( &_s )
          {
            __impl::Thread::gettime( &tm[0] );
            tm[1].tv_sec = 0;
            tm[1].tv_nsec = 0;
          }

        conn( const conn& _c ) :
            s( _c.s )
          { tm[0] = _c.tm[0]; tm[1] = _c.tm[1]; }

        std::sockstream *s;
        timespec tm[2];
    };

    typedef std::pair<std::string,conn> map_type;
    typedef std::map<std::string,conn> container_type;
    typedef std::map<std::sockstream *,std::string> reverse_container_type;
    static __impl::Mutex m_lock;
    static __impl::Mutex r_lock;
    static __impl::Mutex orphan_lock;
    static __impl::Mutex finished_lock;
    static container_type m;
    static reverse_container_type r;
    static container_type orphan;
    static container_type finished;
    std::sockstream& sockstr;
};

#endif // __ConnectionProcessor_h

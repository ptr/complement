// -*- C++ -*- Time-stamp: <99/12/27 18:18:01 ptr>

/*
 *
 * Copyright (c) 1997-1999
 * Petr Ovchenkov
 *
 * Copyright (c) 1999
 * ParallelGraphics Software Systems
 
 * This material is provided "as is", with absolutely no warranty expressed
 * or implied. Any use is at your own risk.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.
 */

#ifndef __NetTransport_h
#define __NetTransport_h

#ident "$SunId$ %Q%"

#ifndef __config_feature_h
#include <config/feature.h>
#endif

#ifndef __SOCKSTREAM__
#include <sockios/sockstream>
#endif

#ifndef __XMT_H
#include <mt/xmt.h>
#endif

#include <string>
#include <sstream>
#include <map>

#ifndef __Event_h
#include <EDS/Event.h>
#endif

#ifndef __EvSession_h
#include <EDS/EvSession.h>
#endif

#ifndef __EventHandler_h
#include <EDS/EventHandler.h>
#endif

#ifndef __EDS_DLL
#  if defined( WIN32 ) && defined( _MSC_VER )
#    define __EDS_DLL __declspec( dllimport )
#  else
#    define __EDS_DLL
#  endif
#endif

namespace EDS {

extern __EDS_DLL void dump( std::ostream&, const EDS::Event& );

class NetTransport_base :
    public EventHandler // to avoid dependence from creation order
{
  public:
    typedef std::map<key_type,key_type> heap_type;
//    typedef std::map<key_type,key_type,std::less<key_type>,
//      __STL_DEFAULT_ALLOCATOR(key_type) > heap_type;

    NetTransport_base() :
        _count( 0 ),
        _sid( badkey ),
        net( 0 ),
        _net_ns( badaddr )
      { }

    NetTransport_base( const char *info ) :
        EventHandler( info ),
        _count( 0 ),
        _sid( badkey ),
        net( 0 ),
        _net_ns( badaddr )
      { }

    __EDS_DLL ~NetTransport_base();

    bool fail() const
      { return net == 0 ? false : net->fail(); }
    bool good() const
      { return net != 0 && net->good(); }
    bool bad() const
      { return net == 0 || net->bad(); }
    bool is_open() const
      { return net != 0 && net->is_open(); }
    __EDS_DLL void close();

    __EDS_DLL
    bool push( const Event& );


    EvSessionManager::key_type sid() const
      { return _sid; }

    addr_type ns() const
      { return _net_ns; }

    static SessionInfo& session_info( const EvSessionManager::key_type& k )
      { return smgr[k]; }
    static void erase_session( const EvSessionManager::key_type& k )
      { smgr.erase( k ); }

  protected:
    addr_type rar_map( addr_type k, const std::string& name );
    bool pop( Event&, SessionInfo& );
    void disconnect();

    std::sockstream *net;
    EvSessionManager::key_type _sid;
    unsigned _count;
    // indeed rar can be inside connect(), but SunPro's CC 5.0
    // to be very huffy about it.
    heap_type rar; // reverce address resolution table
    __impl::Mutex  _lock;
    addr_type _net_ns; // reflection of address of remote name service
    static __EDS_DLL EvSessionManager smgr;
};

class NetTransport :
    public NetTransport_base
{
  public:
    NetTransport() :
        NetTransport_base( "EDS::NetTransport" )
      { }

    __EDS_DLL
    void connect( std::sockstream& );
};

class NetTransportMgr :
    public NetTransport_base
{
  public:
    NetTransportMgr() :
        NetTransport_base( "EDS::NetTransportMgr" )
      { }

    ~NetTransportMgr()
      {
        if ( net ) {
          net->close();
          join();
        }        
        delete net;
        net = 0;
      }

    __EDS_DLL
    addr_type open( const char *hostname, int port,
                    std::sock_base::stype stype = std::sock_base::sock_stream );
    int join()
      { return _thr.join(); }

    __EDS_DLL addr_type make_map( addr_type k, const char *name );

  protected:
    static int _loop( void * );
    __impl::Thread _thr;
};

class NetTransportMP :
    public NetTransport_base
{
  public:
    NetTransportMP() :
        NetTransport_base( "EDS::NetTransportMP" )
      { }

    __EDS_DLL
    void connect( std::sockstream& );
};

} // namespace EDS

#endif // __NetTransport_h

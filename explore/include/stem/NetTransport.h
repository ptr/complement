// -*- C++ -*- Time-stamp: <06/06/29 18:41:17 ptr>

/*
 *
 * Copyright (c) 1997-1999, 2002, 2003, 2005
 * Petr Ovtchenkov
 *
 * Copyright (c) 1999-2001
 * ParallelGraphics Ltd.
 *
 * Licensed under the Academic Free License version 2.1
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

#ifndef __NetTransport_h
#define __NetTransport_h

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

#ifndef __stem_Event_h
#include <stem/Event.h>
#endif

#ifndef __stem_EvSession_h
#include <stem/EvSession.h>
#endif

#ifndef __stem_EventHandler_h
#include <stem/EventHandler.h>
#endif

namespace stem {

extern __FIT_DECLSPEC void dump( std::ostream&, const stem::Event& );

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

    __FIT_DECLSPEC ~NetTransport_base();

    bool fail() const
      { return net == 0 ? false : net->fail(); }
    bool good() const
      { return net != 0 && net->good(); }
    bool bad() const
      { return net == 0 || net->bad(); }
    bool is_open() const
      { return net != 0 && net->is_open(); }
    virtual __FIT_DECLSPEC void close();

    __FIT_DECLSPEC
    bool push( const Event& );


    EvSessionManager::key_type sid() const
      { return _sid; }

    addr_type ns() const
      { return _net_ns; }

    static stem::SessionInfo session_info( const EvSessionManager::key_type& k )
      {
        smgr.lock();
        stem::SessionInfo si( smgr[k] );
        smgr.unlock();
        return si;
      }

    static void session_control( const EvSessionManager::key_type& k,
                                 const stem::addr_type& a )
      {
        smgr.lock();
        smgr[k]._control = a;
        smgr.unlock();
      }

    static std::string session_host( const EvSessionManager::key_type& k )
      {
        smgr.lock();
        std::string h( smgr[k]._host );
        smgr.unlock();
        return h;
      }

    static void erase_session( const EvSessionManager::key_type& k )
      { smgr.erase( k ); }

  protected:
    void establish_session( std::sockstream& s ) throw (std::domain_error);
    void mark_session_onoff( bool );
    addr_type rar_map( addr_type k, const std::string& name );
    bool pop( Event& );
    void disconnect();

    std::sockstream *net;
    EvSessionManager::key_type _sid;
    unsigned _count;
    // indeed rar can be inside connect(), but SunPro's CC 5.0
    // to be very huffy about it.
    heap_type rar; // reverce address resolution table
    addr_type _net_ns; // reflection of address of remote name service
    static __FIT_DECLSPEC EvSessionManager smgr;
};

class NetTransport :
    public NetTransport_base
{
  public:
    __FIT_DECLSPEC
    NetTransport( std::sockstream& );

    __FIT_DECLSPEC
    void connect( std::sockstream& );

  private:
    std::string _at_hostname;
};

class NetTransportMgr :
    public NetTransport_base
{
  public:
    NetTransportMgr() :
        NetTransport_base( "stem::NetTransportMgr" )
      { }

    __FIT_DECLSPEC
    ~NetTransportMgr();

    __FIT_DECLSPEC
    addr_type open( const char *hostname, int port,
                    std::sock_base::stype stype = std::sock_base::sock_stream );
    virtual __FIT_DECLSPEC void close();

    int join()
      { return _thr.join(); }

    __FIT_DECLSPEC addr_type make_map( addr_type k, const char *name );

  protected:
    static int _loop( void * );
    xmt::Thread _thr;
};

class NetTransportMP :
    public NetTransport_base
{
  public:
    NetTransportMP( std::sockstream& s ) :
        NetTransport_base( "stem::NetTransportMP" )
      { this->connect( s ); }

    __FIT_DECLSPEC
    void connect( std::sockstream& );
};

} // namespace stem

#endif // __NetTransport_h

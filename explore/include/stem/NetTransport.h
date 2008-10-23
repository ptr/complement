// -*- C++ -*- Time-stamp: <08/06/30 15:29:44 yeti>

/*
 * Copyright (c) 1997-1999, 2002, 2003, 2005, 2006, 2008
 * Petr Ovtchenkov
 *
 * Copyright (c) 1999-2001
 * ParallelGraphics Ltd.
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __NETTRANSPORT_H
#define __NETTRANSPORT_H

#ifndef __config_feature_h
#include <config/feature.h>
#endif

#include <sockios/sockstream>

#include <mt/thread>

#include <string>
#include <sstream>
#include <map>

#include <stem/Event.h>
#include <stem/EventHandler.h>

namespace stem {

extern __FIT_DECLSPEC void dump( std::ostream&, const stem::Event& );

class NetTransport_base :
    public EventHandler // to avoid dependence from creation order
{
  public:
    NetTransport_base() :
        _count( 0 ),
        net( 0 )
      { }

    NetTransport_base( const char *info ) :
        EventHandler( info ),
        _count( 0 ),
        net( 0 )
      { }

    ~NetTransport_base()
      { NetTransport_base::_close(); }

    bool fail() const
      { return net == 0 ? false : net->fail(); }
    bool good() const
      { return net != 0 && net->good(); }
    bool bad() const
      { return net == 0 || net->bad(); }
    bool is_open() const
      { return net != 0 && net->is_open(); }
    virtual void close()
      { NetTransport_base::_close(); }

    __FIT_DECLSPEC bool push( const Event&, const gaddr_type& dst, const gaddr_type& src );

  protected:
    bool pop( Event&, gaddr_type& dst, gaddr_type& src );
    __FIT_DECLSPEC void _close();

    std::sockstream *net;
    uint32_t _count;
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
    void _do_handshake();
    bool _handshake;
};

class NetTransportMgr :
    public NetTransport_base
{
  public:
    NetTransportMgr() :
        NetTransport_base( "stem::NetTransportMgr" ),
        _thr( 0 )
      { net = &_channel; }

    ~NetTransportMgr()
      { NetTransportMgr::_close(); delete _thr; }

    __FIT_DECLSPEC
    addr_type open( const char *hostname, int port,
                    std::sock_base::stype stype = std::sock_base::sock_stream );
    virtual __FIT_DECLSPEC void close()
      { NetTransportMgr::_close(); }

    void join()
      { if ( _thr != 0 && _thr->joinable() ) { _thr->join(); } }

  private:
    NetTransportMgr( const NetTransportMgr& );
    NetTransportMgr& operator =( const NetTransportMgr& );

  protected:
    __FIT_DECLSPEC void _close();
    static void _loop( NetTransportMgr* );
    std::tr2::thread* _thr;
    std::sockstream _channel;
};

#if 0
class NetTransportMP :
    public NetTransport_base
{
  public:
    NetTransportMP( std::sockstream& s ) :
        NetTransport_base( "stem::NetTransportMP" )
      { net = &s; }

    __FIT_DECLSPEC
    void connect( std::sockstream& );
};
#endif

} // namespace stem

#endif // __NETTRANSPORT_H

// -*- C++ -*- Time-stamp: <08/12/05 11:04:43 ptr>

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
  protected:
    NetTransport_base( std::sockstream& s ) :
        _count( 0 ),
        net( s )
      { }

    NetTransport_base( std::sockstream& s, const char *info ) :
        EventHandler( info ),
        _count( 0 ),
        net( s )
      { }

    ~NetTransport_base()
      { NetTransport_base::_close(); }

  public:
    void close()
      { NetTransport_base::_close(); }

    __FIT_DECLSPEC bool push( const Event&, const gaddr_type& dst, const gaddr_type& src );

  protected:
    bool pop( Event&, gaddr_type& dst, gaddr_type& src );
    __FIT_DECLSPEC void _close();

    uint32_t _count;
    std::sockstream& net;
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
    private std::sockstream,
    public NetTransport_base
{
  public:
    NetTransportMgr() :
        std::sockstream(),
        NetTransport_base( *this, "stem::NetTransportMgr" ),
        _thr( 0 )
      { }

    ~NetTransportMgr()
      { NetTransport_base::_close(); join(); }

    bool fail() const
      { return std::sockstream::fail(); }
    bool good() const
      { return std::sockstream::good(); }
    bool bad() const
      { return std::sockstream::bad(); }
    bool is_open() const
      { return std::sockstream::is_open(); }

    addr_type open( const char *hostname, int port,
                    sock_base::stype type = sock_base::sock_stream,
                    sock_base::protocol pro = sock_base::inet );

    addr_type open( const in_addr& addr, int port,
                    sock_base::stype type = sock_base::sock_stream,
                    sock_base::protocol pro = sock_base::inet );

    addr_type open( sock_base::socket_type s, const sockaddr& addr,
                    sock_base::stype type = sock_base::sock_stream );

    addr_type open( sock_base::socket_type s,
                    sock_base::stype type = sock_base::sock_stream );

    void close()
      {  NetTransport_base::_close(); }

    void join();

  private:
    NetTransportMgr( const NetTransportMgr& );
    NetTransportMgr& operator =( const NetTransportMgr& );

  private:
    addr_type discovery();
    static void _loop( NetTransportMgr* );
    std::tr2::thread* _thr;
};

} // namespace stem

#endif // __NETTRANSPORT_H

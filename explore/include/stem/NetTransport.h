// -*- C++ -*-

/*
 * Copyright (c) 1997-1999, 2002-2003, 2005-2006, 2008-2012, 2020
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

#include <config/feature.h>

#include <sockios/sockstream>

#include <chrono>
#include <thread>

#include <string>
#include <sstream>
#include <map>

#include <stem/Event.h>
#include <stem/EvManager.h>

namespace stem {

extern __FIT_DECLSPEC void dump( std::ostream&, const stem::Event& );

class NetTransport_base
{
  protected:
    struct msg_hdr {
        uint32_t magic;
        uint32_t code;
        uint32_t dstd[4];
        uint32_t srcd[4];
        uint32_t dst[4];
        uint32_t src[4];
        uint32_t flags;
        uint32_t sz;
        uint32_t crc;
    };

  protected:
    NetTransport_base( std::sockstream& s ) :
        net( s ),
        _id( xmt::nil_uuid )
      { }

    NetTransport_base( std::sockstream& s, const char *info ) :
        net( s ),
        _id( xmt::nil_uuid )
      { }

    virtual const std::type_info& classtype() const
       { return typeid(NetTransport_base); }

    virtual int flags() const;

#ifdef __FIT_CPP_0X
    NetTransport_base( NetTransport_base& ) = delete;
    NetTransport_base operator =( const NetTransport_base& ) = delete;
#else
  private:
    NetTransport_base( NetTransport_base& );
    NetTransport_base operator =( const NetTransport_base& );
#endif

  public:
    void close();

    bool Dispatch( const Event& );
    domain_type domain() const;

#if 0
    addr_type ns_remote() const;

    void add_route( const addr_type& );
    void rm_route( const addr_type& );
    void add_remote_route( const addr_type& );
    void rm_remote_route( const addr_type& );
#endif

  protected:
    bool pop( Event& );
    __FIT_DECLSPEC void _close();

    std::sockstream& net;
    stem::EvManager::edge_id_type _id;
};

class NetTransport :
    public NetTransport_base
{
  public:
    __FIT_DECLSPEC
    NetTransport( std::sockstream& );
    virtual ~NetTransport();

    __FIT_DECLSPEC
    void connect( std::sockstream& );

    virtual const std::type_info& classtype() const
       { return typeid(NetTransport); }

  private:
    bool exchange;
};

class NetTransportMgr :
    private std::sockstream,
    public NetTransport_base
{
  public:
    NetTransportMgr() :
        std::sockstream(),
        NetTransport_base( dynamic_cast<std::sockstream&>(*this) ),
        _thr( 0 )
      { }

    NetTransportMgr( const char* info ) :
        std::sockstream(),
        NetTransport_base( dynamic_cast<std::sockstream&>(*this), info ),
        _thr( 0 )
      { }

    ~NetTransportMgr()
      { net.close(); join(); }

    bool fail() const
      { return std::sockstream::fail(); }
    bool good() const
      { return std::sockstream::good(); }
    bool bad() const
      { return std::sockstream::bad(); }
    bool is_open() const
      { return std::sockstream::is_open(); }

    domain_type open( const char* hostname, int port,
                      sock_base::stype type = sock_base::sock_stream,
                      sock_base::protocol pro = sock_base::inet );

    domain_type open( const char* path,
                      sock_base::stype type = sock_base::sock_stream );

    domain_type open( in_addr_t addr, int port,
                      sock_base::stype type = sock_base::sock_stream,
                      sock_base::protocol pro = sock_base::inet );

    domain_type open( const sockaddr_in& addr,
                      sock_base::stype type = sock_base::sock_stream );

    domain_type open( sock_base::socket_type s, const sockaddr& addr,
                      sock_base::stype type = sock_base::sock_stream );

    domain_type open( sock_base::socket_type s,
                      sock_base::stype type = sock_base::sock_stream );

    template <class Duration>
    domain_type open( const char* hostname, int port,
                      const Duration& timeout,
                      sock_base::stype type = sock_base::sock_stream,
                      sock_base::protocol pro = sock_base::inet );

    template <class Duration>
    domain_type open( const char* path, const Duration& timeout,
                      sock_base::stype type = sock_base::sock_dgram );

    template <class Duration>
    domain_type open( in_addr_t addr, int port,
                      const Duration& timeout,
                      sock_base::stype type = sock_base::sock_stream,
                      sock_base::protocol pro = sock_base::inet );

    template <class Duration>
    domain_type open( const sockaddr_in& addr, const Duration& timeout,
                      sock_base::stype type = sock_base::sock_stream );

    void close();

    void join();

    virtual const std::type_info& classtype() const
       { return typeid(NetTransportMgr); }

#ifdef __FIT_CPP_0X
    NetTransportMgr( const NetTransportMgr& ) = delete;
    NetTransportMgr& operator =( const NetTransportMgr& ) = delete;
#else
  private:
    NetTransportMgr( const NetTransportMgr& );
    NetTransportMgr& operator =( const NetTransportMgr& );
#endif

  private:
    domain_type discovery(const std::chrono::nanoseconds& timeout = std::chrono::nanoseconds());

    static void _loop( NetTransportMgr* );
    std::thread* _thr;
};


template <class Duration>
domain_type NetTransportMgr::open( const char* hostname, int port,
                                   const Duration& timeout,
                                   sock_base::stype type,
                                   sock_base::protocol pro )
{
  std::sockstream::open( hostname, port, timeout );
  if ( std::sockstream::is_open() && std::sockstream::good() ) {
    try {
      if ( (pro == sock_base::inet) && (type == sock_base::sock_stream) ) {
        std::sockstream::rdbuf()->setoptions( sock_base::so_tcp_nodelay );
      }
    }
    catch ( ... ) {
    }
    return discovery(std::chrono::duration_cast<std::chrono::nanoseconds>(timeout));
  }
  return stem::baddomain;
}

template <class Duration>
domain_type NetTransportMgr::open( const char* path,
                                   const Duration& timeout,
                                   sock_base::stype type )
{
  std::sockstream::open( path, timeout, type );
  if ( std::sockstream::is_open() && std::sockstream::good() ) {
    return discovery(std::chrono::duration_cast<std::chrono::nanoseconds>(timeout));
  }
  return stem::baddomain;
}

template <class Duration>
domain_type NetTransportMgr::open( in_addr_t addr, int port,
                                   const Duration& timeout,
                                   sock_base::stype type,
                                   sock_base::protocol pro )
{
  std::sockstream::open( addr, port, timeout );
  if ( std::sockstream::is_open() && std::sockstream::good() ) {
    try {
      if ( (pro == sock_base::inet) && (type == sock_base::sock_stream) ) {
        std::sockstream::rdbuf()->setoptions( sock_base::so_tcp_nodelay );
      }
    }
    catch ( ... ) {
    }
    return discovery(std::chrono::duration_cast<std::chrono::nanoseconds>(timeout));
  }
  return stem::baddomain;
}

template <class Duration>
domain_type NetTransportMgr::open( const sockaddr_in& addr,
                                   const Duration& timeout,
                                   sock_base::stype type )
{
  std::sockstream::open( addr, timeout, type );
  if ( std::sockstream::is_open() && std::sockstream::good() ) {
    try {
      if ( type == sock_base::sock_stream ) {
        std::sockstream::rdbuf()->setoptions( sock_base::so_tcp_nodelay );
      }
    }
    catch ( ... ) {
    }
    return discovery(std::chrono::duration_cast<std::chrono::nanoseconds>(timeout));
  }
  return stem::baddomain;
}

} // namespace stem

#endif // __NETTRANSPORT_H

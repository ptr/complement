// -*- C++ -*- Time-stamp: <06/11/24 15:29:19 ptr>

/*
 * Copyright (c) 1997-1999, 2002, 2003, 2005, 2006
 * Petr Ovtchenkov
 *
 * Copyright (c) 1999-2001
 * ParallelGraphics Ltd.
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __NetTransport_h
#define __NetTransport_h

#ifndef __config_feature_h
#include <config/feature.h>
#endif

#include <sockios/sockstream>

#include <mt/xmt.h>

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

    __FIT_DECLSPEC bool push( const Event&, const gaddr_type& dst, const gaddr_type& src );

  protected:
    bool pop( Event&, gaddr_type& dst, gaddr_type& src );

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
      { return _thr.join().iword; }

  protected:
    static xmt::Thread::ret_code _loop( void * );
    xmt::Thread _thr;
};

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

} // namespace stem

#endif // __NetTransport_h

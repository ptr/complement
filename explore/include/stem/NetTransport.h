// -*- C++ -*- Time-stamp: <99/06/04 13:51:26 ptr>

#ifndef __NetTransport_h
#define __NetTransport_h

#ident "$SunId$ %Q%"

#ifndef __SOCKSTREAM__
#include <sockstream>
#endif

#ifndef __XMT_H
#include <xmt.h>
#endif

#include <string>
#include <sstream>
#include <map>

#include <Event.h>
#include <EvSession.h>
#include <EventHandler.h>

namespace EDS {

extern __DLLEXPORT void dump( std::ostream&, const EDS::Event& );

class NetTransport_base :
    public EventHandler // to avoid creation order dependence in manager
{
  public:
    typedef Event::key_type key_type;
    typedef std::map<key_type,key_type,std::less<key_type>,
      __STL_DEFAULT_ALLOCATOR(key_type) > heap_type;

    NetTransport_base() :
        _count( 0 ),
        _sid( -1 ),
        net( 0 )
      { }

    __DLLEXPORT ~NetTransport_base();

    bool fail() const
      { return net == 0 || net->fail(); }
    bool good() const
      { return net != 0 && net->good(); }
    bool is_open() const
      { return net != 0 && net->is_open(); }
    void close()
      { if ( net != 0 ) net->close(); }

    __DLLEXPORT
    bool push( const Event&, const key_type& rmkey, const key_type& srckey );


    EvSessionManager::key_type sid() const
      { return _sid; }

    static SessionInfo& session_info( const EvSessionManager::key_type& k )
      { return smgr[k]; }
    static void erase_session( const EvSessionManager::key_type& k )
      { smgr.erase( k ); }

  protected:
    bool pop( Event&, SessionInfo& );
    void event_process( Event&, const std::string& );
    void disconnect();

    std::sockstream *net;
    EvSessionManager::key_type _sid;
    unsigned _count;
    // indeed rar can be inside connect(), but SunPro's CC 5.0
    // to be very huffy about it.
    heap_type rar; // reverce address resolution table
    __impl::Mutex  _lock;
    static EvSessionManager smgr;
};

class NetTransport :
    public NetTransport_base
{
  public:
    NetTransport()
      { }

    __DLLEXPORT
    void connect( std::sockstream& );
};

class NetTransportMgr :
    public NetTransport_base
{
  public:
    NetTransportMgr()
      { }

    ~NetTransportMgr()
      {
        if ( net ) {
          net->close();
          join();
        }        
        delete net;
      }

    __DLLEXPORT
    key_type open( const char *hostname, int port,
                   std::sock_base::stype stype = std::sock_base::sock_stream );
    int join()
      { return _thr.join(); }

  protected:
    static int _loop( void * );
    __impl::Thread _thr;
};

class NetTransportMP :
    public NetTransport_base
{
  public:
    NetTransportMP()
      { }

    __DLLEXPORT
    void connect( std::sockstream& );
};

} // namespace EDS

#endif // __NetTransport_h

// -*- C++ -*- Time-stamp: <99/09/08 17:30:30 ptr>

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
    typedef Event::key_type key_type;
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
      { return net == 0 || net->fail(); }
    bool good() const
      { return net != 0 && net->good(); }
    bool is_open() const
      { return net != 0 && net->is_open(); }
    void close()
      { if ( net != 0 ) net->close(); }

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
    addr_type rar_map( addr_type k, const string& name );
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
    static EvSessionManager smgr;
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

// -*- C++ -*- Time-stamp: <99/04/16 15:32:04 ptr>

#ifndef __NetTransport_h
#define __NetTransport_h

#ident "%Z% $Date$ $Revision$ $RCSfile$ %Q%"

// To do:
// I think, that no reason assotiate transport with socketstream here;
// one can use abitrary iostream instead. 

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

namespace EDS {

extern __DLLEXPORT void dump( std::ostream&, const EDS::Event& );

class NetTransport_base
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
    bool push( const Event&, const key_type& rmkey,
               const key_type& srckey, const Event::key_type& sid );


    EvSessionManager::key_type sid() const
      { return _sid; }

    static SessionInfo& session_info( const EvSessionManager::key_type& k )
      { return smgr[k]; }
    static void erase_session( const EvSessionManager::key_type& k )
      { smgr.erase( k ); }

  protected:
    bool pop( Event& );
    void event_process( Event&, SessionInfo&, const std::string& );
    void disconnect();

    std::sockstream *net;
    EvSessionManager::key_type _sid;
    unsigned _count;
    // indeed rar can be inside connect(), but SunPro's CC 5.0
    // to be very huffy about it.
    heap_type rar; // reverce address resolution table
    // __impl::Mutex  _lock;
    static EvSessionManager smgr;
};

class NetTransport :
    public NetTransport_base
{
  public:
    NetTransport()
      { }

    __DLLEXPORT
    void connect( std::sockstream&, const std::string& hostname,
                  std::string& info );
};

class NetTransportMgr :
    public NetTransport_base
{
  public:
    NetTransportMgr()
      { }

    ~NetTransportMgr()
      { delete net; }

    __DLLEXPORT
    key_type open( const std::string& hostname, int port,
                   std::sock_base::stype stype = std::sock_base::sock_stream );
    int join()
      { return _thr.join(); }

  protected:
    static int _loop( void * );
    __impl::Thread _thr;

    std::string _partner_name;
};

class NetTransportMP :
    public NetTransport_base
{
  public:
    NetTransportMP()
      { }

    __DLLEXPORT
    void connect( std::sockstream&, const std::string& hostname,
                  std::string& info );
};

} // namespace EDS

#endif // __NetTransport_h

// -*- C++ -*- Time-stamp: <99/03/26 20:19:09 ptr>

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

class NetTransport
{
  public:
    typedef Event::key_type key_type;
    typedef std::map<key_type,key_type,std::less<key_type>,
      __STL_DEFAULT_ALLOCATOR(key_type) > heap_type;

    NetTransport() :
        _count( 0 ),
        _sid( -1 ),
        net( 0 ),
        _net_owner( false )
      { }

    __DLLEXPORT ~NetTransport();

    __DLLEXPORT
    key_type open( const std::string& hostname, int port );
    __DLLEXPORT
    void connect( std::sockstream&, const std::string& hostname,
                  std::string& info );
    __DLLEXPORT
    bool push( const Event&, const key_type& rmkey, const key_type& srckey );

    bool fail() const
      { return net == 0 || net->fail(); }
    bool good() const
      { return net != 0 && net->good(); }
    bool is_open() const
      { return net != 0 && net->is_open(); }

    void close()
      {
        if ( net != 0 ) {
          net->close();
        }
      }

    EvSessionManager::key_type sid() const
      { return _sid; }

  private:
    bool pop( Event& );

    std::sockstream *net;
    EvSessionManager::key_type _sid;
    unsigned _count;
    // indeed rar can be inside connect(), but SunPro's CC 5.0
    // to be very huffy about it.
    heap_type rar; // reverce address resolution table

    std::string _partner_name;
    static int _loop( void * );
    __impl::Thread _thr;
    // __impl::Mutex  _lock;
    bool _net_owner;
};


} // namespace EDS

#endif // __NetTransport_h

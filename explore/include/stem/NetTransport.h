// -*- C++ -*- Time-stamp: <99/03/19 18:26:03 ptr>

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

namespace EDS {

extern __DLLEXPORT void dump( std::ostream&, const EDS::Event& );

class NetTransport
{
  public:
    typedef Event::key_type key_type;
    typedef std::map<key_type,key_type,std::less<key_type>,
      __STL_DEFAULT_ALLOCATOR(key_type) > heap_type;

    __DLLEXPORT NetTransport() :
        _count( 0 )
      { }

    __DLLEXPORT
    void connect( std::sockstream&, const std::string& hostname,
                  std::string& info );
    __DLLEXPORT
    bool push( const Event&, const key_type& rmkey, const key_type& srckey );

  private:
    bool pop( Event& );

    std::sockstream *net;
    unsigned _count;
    // indeed rar can be inside connect(), but SunPro's CC 5.0
    // to be very huffy about it.
    heap_type rar; // reverce address resolution table
};


} // namespace EDS

#endif // __NetTransport_h

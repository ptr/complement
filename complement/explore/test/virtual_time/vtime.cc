// -*- C++ -*- Time-stamp: <07/03/07 15:53:23 ptr>

#include "vtime.h"

#include <stdint.h>

namespace vt {

using namespace std;
using namespace xmt;
using namespace stem;

void vtime::pack( std::ostream& s ) const
{
  __pack( s, static_cast<uint8_t>(vt.size()) );
  for ( vtime_type::const_iterator i = vt.begin(); i != vt.end(); ++i ) {
    __pack( s, i->first );
    __pack( s, i->second );
  }
}

void vtime::net_pack( std::ostream& s ) const
{
  __net_pack( s, static_cast<uint8_t>(vt.size()) );
  for ( vtime_type::const_iterator i = vt.begin(); i != vt.end(); ++i ) {
    __net_pack( s, i->first );
    __net_pack( s, i->second );
  }
}

void vtime::unpack( std::istream& s )
{
  vt.clear();
  uint8_t n;
  __unpack( s, n );
  while ( n-- > 0 ) {
    vtime_proc_type v;

    __unpack( s, v.first );
    __unpack( s, v.second );

    vt.push_back( v );
  }
}

void vtime::net_unpack( std::istream& s )
{
  vt.clear();
  uint8_t n;
  __net_unpack( s, n );
  while ( n-- > 0 ) {
    vtime_proc_type v;

    __net_unpack( s, v.first );
    __net_unpack( s, v.second );

    vt.push_back( v );
  }
}

void Proc::mess( const stem::Event_base<vtime>& ev )
{
}

DEFINE_RESPONSE_TABLE( Proc )
  EV_Event_base_T_( ST_NULL, MESS, mess, vtime )
END_RESPONSE_TABLE

} // namespace vt

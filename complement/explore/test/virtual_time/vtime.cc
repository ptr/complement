// -*- C++ -*- Time-stamp: <07/05/17 23:30:42 ptr>

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

vtime_type operator -( const vtime_type& l, const vtime_type& r )
{
  if ( r.empty() ) {
    return l;
  }

  vtime_type vt;
  vtime_type::const_iterator i = l.begin();
  vtime_type::const_iterator j = r.begin();

  while ( i != l.end() && j != r.end() ) {
    while ( i->first < j->first && i != l.end() ) {
      vt.push_back( make_pair( i->first, i->second ) );
      ++i;
    }
  
    while ( i->first == j->first && i != l.end() && j != r.end() ) {
      if ( i->second < j->second ) {
        throw range_error( "vtime different: right value grater then left" );
      }
      vt.push_back( make_pair( i->first, i->second - j->second ) );
      ++i;
      ++j;
    }
  }

  if ( i == l.end() && j != r.end() ) {
    throw range_error( "vtime different: right value grater then left" );
  }

  while ( i != l.end() ) {
    vt.push_back( make_pair( i->first, i->second ) );
    ++i;    
  }

  return vt;
}


void Proc::mess( const stem::Event_base<vtime>& ev )
{
}

DEFINE_RESPONSE_TABLE( Proc )
  EV_Event_base_T_( ST_NULL, MESS, mess, vtime )
END_RESPONSE_TABLE

} // namespace vt

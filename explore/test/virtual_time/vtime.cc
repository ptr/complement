// -*- C++ -*- Time-stamp: <07/05/24 10:24:22 ptr>

#include "vtime.h"

#include <iostream>
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
    oid_type oid;
    vtime_unit_type v;

    __unpack( s, oid );
    __unpack( s, v );

    vt[oid] = v;
  }
}

void vtime::net_unpack( std::istream& s )
{
  vt.clear();
  uint8_t n;
  __net_unpack( s, n );
  while ( n-- > 0 ) {
    oid_type oid;
    vtime_unit_type v;

    __net_unpack( s, oid );
    __net_unpack( s, v );

    vt[oid] = v;
  }
}

void gvtime::pack( std::ostream& s ) const
{
  __pack( s, static_cast<uint8_t>(gvt.size()) );
  for ( gvtime_type::const_iterator i = gvt.begin(); i != gvt.end(); ++i ) {
    __pack( s, i->first );
    i->second.pack( s );
  }
}

void gvtime::net_pack( std::ostream& s ) const
{
  __net_pack( s, static_cast<uint8_t>(gvt.size()) );
  for ( gvtime_type::const_iterator i = gvt.begin(); i != gvt.end(); ++i ) {
    __net_pack( s, i->first );
    i->second.net_pack( s );
  }
}

void gvtime::unpack( std::istream& s )
{
  gvt.clear();
  uint8_t n;
  __unpack( s, n );
  while ( n-- > 0 ) {
    group_type gid;
    __unpack( s, gid );
    gvt[gid].unpack( s );
  }
}

void gvtime::net_unpack( std::istream& s )
{
  gvt.clear();
  uint8_t n;
  __net_unpack( s, n );
  while ( n-- > 0 ) {
    group_type gid;
    __net_unpack( s, gid );
    gvt[gid].net_unpack( s );
  }
}

void VTmess::pack( std::ostream& s ) const
{
  gvt.pack( s );
  __pack( s, grp );
  __pack( s, mess );
}

void VTmess::net_pack( std::ostream& s ) const
{
  gvt.net_pack( s );
  __net_pack( s, grp );
  __net_pack( s, mess );
}

void VTmess::unpack( std::istream& s )
{
  gvt.unpack( s );
  __unpack( s, grp );
  __unpack( s, mess );
}

void VTmess::net_unpack( std::istream& s )
{
  gvt.net_unpack( s );
  __net_unpack( s, grp );
  __net_unpack( s, mess );
}

bool operator <=( const vtime_type& l, const vtime_type& r )
{
  if ( r.empty() ) {
    return l.empty();
  }

  if ( l.empty() ) {
    return true;
  }

  vtime_type::const_iterator i = l.begin();
  vtime_type::const_iterator j = r.begin();

  while ( i != l.end() && j != r.end() ) {
    if ( i->first < j->first ) { // v <= 0 --- false
      return false;
    }
    while ( j != r.end() && i->first > j->first ) { // 0 <= v --- true
      ++j;
    }
  
    while ( i != l.end() && j != r.end() && i->first == j->first ) {
      if ( i->second > j->second ) {
	return false;
      }
      ++i;
      ++j;
    }
  }

  if ( i == l.end() ) {
    return true;
  }

  return false;
}

vtime_type operator -( const vtime_type& l, const vtime_type& r )
{
  vtime_type tmp( r.begin(), r.end() );

  for ( vtime_type::iterator i = tmp.begin(); i != tmp.end(); ++i ) {
    vtime_type::const_iterator p = l.find(i->first);
    if ( p == l.end() || p->second < i->second ) {
      throw range_error( "vtime different: right value grater then left" );
    }
    i->second = p->second - i->second;
  }

  for ( vtime_type::const_iterator i = l.begin(); i != l.end(); ++i ) {
    vtime_type::iterator p = tmp.find(i->first);
    if ( p == tmp.end() ) {
      tmp[i->first] = i->second;
    }
  }

  return tmp;
}

vtime_type operator +( const vtime_type& l, const vtime_type& r )
{
  vtime_type tmp( l.begin(), l.end() );

  for ( vtime_type::const_iterator i = r.begin(); i != r.end(); ++i ) {
    tmp[i->first] += i->second;
  }

  return tmp;
}

vtime_type& operator +=( vtime_type& l, const vtime_type& r )
{
  for ( vtime_type::const_iterator i = r.begin(); i != r.end(); ++i ) {
    l[i->first] += i->second;
  }

  return l;
}

// template <>
vtime_type max( const vtime_type& l, const vtime_type& r )
{
  vtime_type tmp( l.begin(), l.end() );

  for ( vtime_type::const_iterator i = r.begin(); i != r.end(); ++i ) {
    tmp[i->first] = std::max( tmp[i->first], i->second );
  }
  return tmp;
}

vtime& vtime::operator +=( const vtime_type::value_type& t )
{
  vt[t.first] += t.second;

  return *this;
}

gvtime_type& operator +=( gvtime_type& gvt, gvtime_type::value_type& t )
{
  gvt[t.first] += t.second;

  return gvt;
}

gvtime& gvtime::operator +=( const gvtime_type::value_type& t )
{
  gvt[t.first] += t.second;

  return *this;  
}

vtime_unit_type comp( const gvtime_type& gvt, group_type g, oid_type p )
{
  gvtime_type::const_iterator k = gvt.begin();
  for ( ; k != gvt.end(); ++k ) {
    if ( k->first == g ) {
      for ( vtime_type::const_iterator i = k->second.vt.begin(); i != k->second.vt.end(); ++i ) {
	if ( i->first == p ) {
	  return i->second; // found (p_i, t) in g_k
	} else if ( i->first > p ) {
	  return 0; // pair sorted, so no pair required (p_i, t) expected more
	}
      }
      return 0; // no pair (p_i, * )
    } else if ( k->first > g ) { // groups sorted, so no required group expected more
      return 0;
    }
  }

  return 0;
}


void Proc::mess( const stem::Event_base<VTmess>& ev )
{
  cout << ev.value().mess << endl;

  cout << ev.value().gvt.gvt << endl;

  cout << order_correct( ev ) << endl;

  if ( order_correct( ev ) ) {
    gvtime_type::const_iterator gr = ev.value().gvt.gvt.begin();
    gvtime_type::const_iterator ge = ev.value().gvt.gvt.end();
    for ( ; gr != ge; ++gr ) { // over all groups
      // vt[gr->first] = max( vt[gr->first], vt[gr->first] + gr->second.vt );
    }
  }
}

bool Proc::order_correct( const stem::Event_base<VTmess>& ev )
{
  return false;
}

DEFINE_RESPONSE_TABLE( Proc )
  EV_Event_base_T_( ST_NULL, MESS, mess, VTmess )
END_RESPONSE_TABLE

} // namespace vt

namespace std {

ostream& operator <<( ostream& o, const vt::vtime_type::value_type& v )
{
  return o << "(" << v.first << "," << v.second << ")\n";
}

ostream& operator <<( ostream& o, const vt::vtime_type& v )
{
  o << "[\n";
  for ( vt::vtime_type::const_iterator i = v.begin(); i != v.end(); ++i ) {
    o << *i;
  }
  return o << "]\n";
}

ostream& operator <<( ostream& o, const vt::gvtime_type::value_type& v )
{
  o << v.first << ": " << v.second.vt;
}

ostream& operator <<( ostream& o, const vt::gvtime_type& v )
{
  o << "{\n";
  for ( vt::gvtime_type::const_iterator i = v.begin(); i != v.end(); ++i ) {
    o << *i;
  }
  return o << "}\n";
}

} // namespace std


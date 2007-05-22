// -*- C++ -*- Time-stamp: <07/05/17 23:30:42 ptr>

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
    gvt.push_back( vtime_group_type() );
    __unpack( s, gvt.back().first );
    gvt.back().second.unpack( s );
  }
}

void gvtime::net_unpack( std::istream& s )
{
  gvt.clear();
  uint8_t n;
  __net_unpack( s, n );
  while ( n-- > 0 ) {
    gvt.push_back( vtime_group_type() );
    __net_unpack( s, gvt.back().first );
    gvt.back().second.net_unpack( s );
  }
}

void VTmess::pack( std::ostream& s ) const
{
  gvt.pack( s );
  __pack( s, mess );
}

void VTmess::net_pack( std::ostream& s ) const
{
  gvt.net_pack( s );
  __net_pack( s, mess );
}

void VTmess::unpack( std::istream& s )
{
  gvt.unpack( s );
  __unpack( s, mess );
}

void VTmess::net_unpack( std::istream& s )
{
  gvt.net_unpack( s );
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
  if ( r.empty() ) {
    return l;
  }

  vtime_type vt;
  vtime_type::const_iterator i = l.begin();
  vtime_type::const_iterator j = r.begin();

  while ( i != l.end() && j != r.end() ) {
    while ( i != l.end() && i->first < j->first ) {
      vt.push_back( make_pair( i->first, i->second ) );
      ++i;
    }
  
    while ( i != l.end() && j != r.end() && i->first == j->first ) {
      if ( i->second < j->second ) {
        throw range_error( "vtime different: right value grater then left" );
      } else if ( i->second > j->second ) {
	vt.push_back( make_pair( i->first, i->second - j->second ) );
      }
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

vtime_type operator +( const vtime_type& l, const vtime_type& r )
{
  if ( r.empty() ) {
    return l;
  }

  if ( l.empty() ) {
    return r;
  }

  vtime_type vt;
  vtime_type::const_iterator i = l.begin();
  vtime_type::const_iterator j = r.begin();

  while ( i != l.end() && j != r.end() ) {
    while ( i != l.end() && i->first < j->first ) {
      vt.push_back( make_pair( i->first, i->second ) );
      ++i;
    }
  
    while ( i != l.end() && j != r.end() && i->first == j->first ) {
      vt.push_back( make_pair( i->first, i->second + j->second ) );
      ++i;
      ++j;
    }
  }

  while ( i != l.end() ) {
    vt.push_back( make_pair( i->first, i->second ) );
    ++i;
  }
  while ( j != r.end() ) {
    vt.push_back( make_pair( j->first, j->second ) );
    ++j;
  }

  return vt;
}

// template <>
vtime_type max( const vtime_type& l, const vtime_type& r )
{
  if ( l.empty() ) {
    return r;
  }

  if ( r.empty() ) {
    return l;
  }

  // here l and r non-empty

  vtime_type vt;
  vtime_type::const_iterator i = l.begin();
  vtime_type::const_iterator j = r.begin();

  while ( i != l.end() && j != r.end() ) {
    while ( i != l.end() && i->first < j->first ) {
      vt.push_back( make_pair( i->first, i->second ) );
      ++i;
    }

    while ( i != l.end() && j != r.end() && i->first == j->first ) {
      vt.push_back( make_pair( i->first, std::max( i->second, j->second ) ) );
      ++i;
      ++j;
    }
  }
  while ( i != l.end() ) {
    vt.push_back( make_pair( i->first, i->second ) );
    ++i;
  }
  while ( j != r.end() ) {
    vt.push_back( make_pair( j->first, j->second ) );
    ++j;
  }

  return vt;
}

void Proc::mess( const stem::Event_base<VTmess>& ev )
{
  cout << ev.value().mess << endl;
}

bool Proc::order_correct( const stem::Event_base<VTmess>& ev )
{
  // assume here first_group

  gvtime_type::const_iterator gr = ev.value().gvt.gvt.begin();
  gvtime_type::const_iterator ge = ev.value().gvt.gvt.end();

  for ( ; gr != ge; ++gr ) {
    if ( gr->first == first_group ) {
      vtime_type vt_tmp = last_vt[first_group] + gr->second.vt;

      vtime_type::const_iterator i = vt_tmp.begin();
      vtime_type::const_iterator j = vt[first_group].begin();

      while ( i != vt_tmp.end() && j != vt[first_group].end() ) {
	if ( i->first < j->first ) {
          return false; // really protocol fail: group member was lost
	}
	
	while ( i->first == j->first && i != vt_tmp.end() && j != vt[first_group].end() ) {
	  if ( i->first == self_id() ) {
	    if ( i->second != (j->second + 1) ) {
	      return false;
	    }
	  } else {
	    if ( i->second > j->second ) {
	      return false;
	    }
	  }
	  ++i;
	  ++j;
	}

	if ( i != vt_tmp.end() ) {
	  return false; // really protocol fail: group member lost
	}

	while ( j != vt[first_group].end() ) {
	  if ( j->first == self_id() && j->second != 1 ) {
	    return false;
	  }
	  ++j;
	}
      }
    } else {
      vtime_type vt_tmp = last_vt[second_group] + gr->second.vt;
      if ( !(vt_tmp <= vt[second_group] )) {
	return false;
      }
    }
  }
}

DEFINE_RESPONSE_TABLE( Proc )
  EV_Event_base_T_( ST_NULL, MESS, mess, VTmess )
END_RESPONSE_TABLE

} // namespace vt

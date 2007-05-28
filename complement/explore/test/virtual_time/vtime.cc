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
    if ( i != l.end() ) {
      while ( j != r.end() && j->first < i->first ) {
	vt.push_back( make_pair( j->first, j->second ) );
	++j;
      }
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

vtime_type& operator +=( vtime_type& l, const vtime_type& r )
{
  if ( r.empty() ) {
    return l;
  }

  if ( l.empty() ) {
    l = r;
    return l;
  }

  vtime_type::iterator i = l.begin();
  vtime_type::const_iterator j = r.begin();

  while ( i != l.end() && j != r.end() ) {
    while ( i != l.end() && i->first < j->first ) {
      ++i;
    }
  
    while ( i != l.end() && j != r.end() && i->first == j->first ) {
      i->second += j->second;
      ++i;
      ++j;
    }

    while ( i != l.end() && j != r.end() && j->first < i->first ) {
      l.insert( i, make_pair( j->first, j->second ) );
      ++i;
      ++j;
    }
  }

  while ( j != r.end() ) {
    l.push_back( make_pair( j->first, j->second ) );
    ++j;
  }

  return l;
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

vtime& vtime::operator += ( const vtime_proc_type& t )
{
  vtime_type::iterator i = vt.begin();

  for ( ; i != vt.end(); ++i ) {
    if ( i->first > t.first ) {
      break;
    } else if ( i->first == t.first ) {
      i->second += t.second;
      return *this;
    }
  }
  vt.insert( i, t );
  return *this;
}

gvtime_type& operator +=( gvtime_type& gvt, const vtime_group_type& t )
{
  gvtime_type::iterator i = gvt.begin();

  for ( ; i != gvt.end(); ++i ) {
    if ( i->first > t.first ) {
      break;
    } else if ( i->first == t.first ) {
      i->second += t.second;
      return gvt;
    }
  }
  gvt.insert( i, t );
  return gvt;
}

gvtime& gvtime::operator +=( const vtime_group_type& t )
{
  gvt += t;
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
  gvtime_type::const_iterator gr = ev.value().gvt.gvt.begin();
  gvtime_type::const_iterator ge = ev.value().gvt.gvt.end();

  bool fine = false;

  group_type mgrp = ev.value().grp;

  for ( ; gr != ge; ++gr ) { // over all groups
    if ( gr->first == mgrp ) {
#if 0
      comp( vt, mgrp, ev.src() );
      comp( vt, mgrp, ev.src() );

      for ( gvtime_type::const_iterator my_gi = vt.begin(); my_gi != vt.end(); ++my_gi ) {
	if ( my_gi->first == mgrp ) {
	  vtime_type vt_tmp = my_gi->second.vt + gr->second.vt;
	  for (vtime_type::const_iterator i = vt_tmp.begin(); i != vt_tmp.end(); ++i ) {
	    if ( i->first == ev.src() ) {
	      vtime_type::const_iterator j = my_gi->second.vt.begin();
	      for ( ; j != my_gi->second.vt.end(); ++j ) {
		if ( i->first == j->first ) {
		  if ( i->second != j->second + 1) {
		    return false;
		  }
		  break;
		}
	      }
	      if ( j == my_gi->second.vt.end() ) {
		if ( i->second != 1 ) {
		  return false;
		}
	      }
	    } else {
	      if ( ) {
	      }
	    }
	  }
	  vtime_type vt_null;
	  vt_null += make_pair( ev.src(), 1 );
	}
      }
      // vtime_group_type vt_tmp = vt + *gr;
      // vtime_type vt_tmp = last_vt[ev.value().grp] + gr->second.vt;
#endif

#if 0
      vtime_type::const_iterator i = vt_tmp.begin();
      if ( vt[mgrp].empty() ) {
	vtime vt_null;
	vt_null += make_pair( ev.src(), 1 );
	cerr << vt_tmp << vt_null.vt;
	return vt_tmp == vt_null.vt;
      } else {
	vtime_type::const_iterator j = vt[mgrp].begin();
      }
#endif
    } else {
#if 0
      vtime_type vt_tmp = last_vt[mgrp] + gr->second.vt;
      if ( !(vt_tmp <= vt[mgrp] )) {
	return false;
      }
#endif
    }
  }

  return fine;
}

DEFINE_RESPONSE_TABLE( Proc )
  EV_Event_base_T_( ST_NULL, MESS, mess, VTmess )
END_RESPONSE_TABLE

} // namespace vt

namespace std {

ostream& operator <<( ostream& o, const vt::vtime_proc_type& v )
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

ostream& operator <<( ostream& o, const vt::vtime_group_type& v )
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


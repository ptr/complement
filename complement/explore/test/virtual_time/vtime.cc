// -*- C++ -*- Time-stamp: <07/07/26 10:28:55 ptr>

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
  __pack( s, code );
  __pack( s, src );
  gvt.pack( s );
  __pack( s, grp );
  __pack( s, mess );
}

void VTmess::net_pack( std::ostream& s ) const
{
  __net_pack( s, code );
  __net_pack( s, src );
  gvt.net_pack( s );
  __net_pack( s, grp );
  __net_pack( s, mess );
}

void VTmess::unpack( std::istream& s )
{
  __unpack( s, code );
  __unpack( s, src );
  gvt.unpack( s );
  __unpack( s, grp );
  __unpack( s, mess );
}

void VTmess::net_unpack( std::istream& s )
{
  __net_unpack( s, code );
  __net_unpack( s, src );
  gvt.net_unpack( s );
  __net_unpack( s, grp );
  __net_unpack( s, mess );
}

bool operator <=( const vtime_type& l, const vtime_type& r )
{
  if ( l.empty() ) {
    return true;
  }

  for ( vtime_type::const_iterator i = l.begin(); i != l.end(); ++i ) {
    if ( i->second > 0 ) {
      vtime_type::const_iterator j = r.find( i->first );
      if ( j == r.end() || i->second > j->second ) {
	return false;
      }
    }
  }

  return true;
}

vtime_type operator -( const vtime_type& l, const vtime_type& r )
{
  vtime_type tmp( r.begin(), r.end() );

  for ( vtime_type::iterator i = tmp.begin(); i != tmp.end(); ++i ) {
    if ( i->second > 0 ) {
      vtime_type::const_iterator p = l.find(i->first);
      if ( p == l.end() || p->second < i->second ) {
	throw range_error( "vtime different: right value grater then left" );
      }
      i->second = p->second - i->second;
    }
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

#if 0
// template <>
vtime_type max( const vtime_type& l, const vtime_type& r )
{
  vtime_type tmp( l.begin(), l.end() );

  for ( vtime_type::const_iterator i = r.begin(); i != r.end(); ++i ) {
    tmp[i->first] = std::max( tmp[i->first], i->second );
  }
  return tmp;
}
#endif

vtime_type& sup( vtime_type& l, const vtime_type& r )
{
  for ( vtime_type::const_iterator i = r.begin(); i != r.end(); ++i ) {
    l[i->first] = std::max( l[i->first], i->second );
  }
  return l;
}


#if 0
// template <>
vtime max( const vtime& l, const vtime& r )
{
  vtime tmp( l );

  for ( vtime_type::const_iterator i = r.vt.begin(); i != r.vt.end(); ++i ) {
    tmp[i->first] = std::max( tmp[i->first], i->second );
  }
  return tmp;
}
#endif

vtime& sup( vtime& l, const vtime& r )
{
  for ( vtime_type::const_iterator i = r.vt.begin(); i != r.vt.end(); ++i ) {
    l[i->first] = std::max( l[i->first], i->second );
  }
  return l;
}

vtime& vtime::operator +=( const vtime_type::value_type& t )
{
  vt[t.first] += t.second;

  return *this;
}

gvtime_type& operator +=( gvtime_type& gvt, const gvtime_type::value_type& t )
{
  gvt[t.first] += t.second;

  return gvt;
}

gvtime_type& operator +=( gvtime_type& l, const gvtime_type& r )
{
  for ( gvtime_type::const_iterator g = r.begin(); g != r.end(); ++g ) {
    l[g->first] += g->second;
  }

  return l;
}

gvtime_type operator -( const gvtime_type& l, const gvtime_type& r )
{
  gvtime_type tmp( r );

  for ( gvtime_type::iterator g = tmp.begin(); g != tmp.end(); ++g ) {
    gvtime_type::const_iterator i = l.find( g->first );
    if ( i == l.end() ) {
      throw range_error( "gvtime different: right value grater then left" );
    }

    g->second = i->second - g->second;
  }

  for ( gvtime_type::const_iterator g = l.begin(); g != l.end(); ++g ) {
    gvtime_type::const_iterator i = tmp.find(g->first);
    if ( i == tmp.end() ) {
      tmp[g->first] = g->second;
    }
  }

  return tmp;
}

gvtime& gvtime::operator +=( const gvtime_type::value_type& t )
{
  gvt[t.first] += t.second;

  return *this;  
}

gvtime& gvtime::operator +=( const gvtime& t )
{
  for ( gvtime_type::const_iterator g = t.gvt.begin(); g != t.gvt.end(); ++g ) {
    gvt[g->first] += g->second;
  }

  return *this;
}

namespace detail {

bool vtime_obj_rec::deliver( const VTmess& m )
{
  if ( order_correct( m ) ) {
    lvt[m.src] += m.gvt.gvt;
    lvt[m.src][m.grp][m.src] = vt.gvt[m.grp][m.src] + 1;
    sup( vt.gvt[m.grp], lvt[m.src][m.grp] );
    // cout << vt.gvt << endl;
    return true;
  }

  return false;
}

bool vtime_obj_rec::deliver_delayed( const VTmess& m )
{
  if ( order_correct_delayed( m ) ) {
    lvt[m.src] += m.gvt.gvt;
    lvt[m.src][m.grp][m.src] = vt.gvt[m.grp][m.src] + 1;
    sup( vt.gvt[m.grp], lvt[m.src][m.grp] );
    // cout << vt.gvt << endl;
    return true;
  }

  return false;
}

bool vtime_obj_rec::order_correct( const VTmess& m )
{
  if ( groups.find( m.grp ) == groups.end() ) {
    throw domain_error( "VT object not member of group" );
  }

  gvtime gvt( m.gvt );

  if ( (vt.gvt[m.grp][m.src] + 1) != gvt[m.grp][m.src] ) {
    // cerr << "1" << endl;
    // cerr << vt.gvt[m.grp][m.src] << "\n"
    //      << gvt[m.grp][m.src]
    //      << endl;
    if ( (vt.gvt[m.grp][m.src] + 1) > gvt[m.grp][m.src] ) {
      throw out_of_range( "duplicate or wrong VT message" );
    }
    return false;
  }

  vtime xvt = lvt[m.src][m.grp] + gvt[m.grp];
  xvt[m.src] = 0; // force exclude message originator, it checked above

  if ( !(xvt <= vt[m.grp]) ) {
    // cerr << "2" << endl;
    // cerr << xvt << "\n\n" << vt[m.grp] << endl;
    return false;
  }

  // check side casuality (via groups other then message's group)
  for ( groups_container_type::const_iterator l = groups.begin(); l != groups.end(); ++l ) {
    if ( (*l != m.grp) && !((lvt[m.src][*l] + gvt[*l]) <= vt[*l]) ) {
      // cerr << "3" << endl;
      // cerr << "group " << *l << xvt << "\n\n" << vt[*l] << endl;
      return false;
    }
  }

  return true;
}

bool vtime_obj_rec::order_correct_delayed( const VTmess& m )
{
  gvtime gvt( m.gvt );

  if ( (vt.gvt[m.grp][m.src] + 1) != gvt[m.grp][m.src] ) {
    return false;
  }

  vtime xvt = lvt[m.src][m.grp] + gvt[m.grp];
  xvt[m.src] = 0; // force exclude message originator, it checked above

  if ( !(xvt <= vt[m.grp]) ) {
    return false;
  }

  // check side casuality (via groups other then message's group)
  for ( groups_container_type::const_iterator l = groups.begin(); l != groups.end(); ++l ) {
    if ( (*l != m.grp) && !((lvt[m.src][*l] + gvt[*l]) <= vt[*l]) ) {
      return false;
    }
  }

  return true;
}

} // namespace detail

void VTDispatcher::VTDispatch( const stem::Event_base<VTmess>& m )
{
  pair<gid_map_type::const_iterator,gid_map_type::const_iterator> range =
    grmap.equal_range( m.value().grp );

  for ( ; range.first != range.second; ++range.first ) {
    vt_map_type::iterator i = vtmap.find( range.first->second );
    if ( i == vtmap.end() || i->second.addr == m.src() ) { // not for nobody and not for self
      continue;
    }
    try {
      // check local or remote? i->second.addr
      // if remote, forward it to foreign VTDispatcher?
      // looks, like local source shouldn't be here?
      check_and_send( i, m );
    }
    catch ( const out_of_range& ) {
    }
    catch ( const domain_error& ) {
    }
  }
}

void VTDispatcher::check_and_send( const vt_map_type::iterator& i, const stem::Event_base<VTmess>& m )
{
  typedef detail::vtime_obj_rec::dpool_t dpool_t;

  if ( i->second.deliver( m.value() ) ) {
    stem::Event ev( m.value().code );
    ev.dest(i->second.addr);
    ev.src(m.src());
    ev.value() = m.value().mess;
    Forward( ev );
    bool more;
    do {
      more = false;
      for ( dpool_t::iterator j = i->second.dpool.begin(); j != i->second.dpool.end(); ) {
        if ( i->second.deliver_delayed( j->second->value() ) ) {
          stem::Event evd( j->second->value().code );
          evd.dest(i->second.addr);
          ev.src(m.src());
          evd.value() = j->second->value().mess;
          Forward( evd );
          delete j->second;
          i->second.dpool.erase( j++ );
          more = true;
        } else {
          ++j;
        }
      }
    } while ( more );
  } else {
    i->second.dpool.push_back( make_pair( 0, new Event_base<VTmess>(m) ) ); // 0 should be timestamp
  }
}

void VTDispatcher::VTSend( const stem::Event& e, group_type grp )
{
  const pair<gid_map_type::const_iterator,gid_map_type::const_iterator> range =
    grmap.equal_range( grp );

  for ( gid_map_type::const_iterator o = range.first; o != range.second; ++o ) {
    vt_map_type::iterator i = vtmap.find( o->second );
    if ( i != vtmap.end() && i->second.addr == e.src() ) { // for self
      stem::Event_base<VTmess> m( MESS );
      m.value().src = o->second; // oid
      m.value().code = e.code();
      m.value().mess = e.value();
      m.value().grp = grp;
      // m.dest( ??? ); // local VT dispatcher?
      m.src( e.src() );

      // This is like VTDispatch, but VT stamp in every message different,
      // in accordance with individual knowlage about object's VT.

      ++i->second.vt.gvt[grp][o->second]; // my counter

      for ( gid_map_type::const_iterator g = range.first; g != range.second; ++g ) {
        vt_map_type::iterator k = vtmap.find( g->second );
        if ( k == vtmap.end() || k->second.addr == m.src() ) { // not for nobody and not for self
          continue;
        }
        try {
          m.value().gvt.gvt = i->second.vt.gvt - i->second.svt[g->second]; // delta
          m.value().gvt[grp][o->second] = i->second.vt.gvt[grp][o->second]; // my counter, as is, not delta

          // check local or remote? i->second.addr
          // if remote, forward it to foreign VTDispatcher?
          check_and_send( k, m );

          i->second.svt[g->second] = i->second.vt.gvt; // store last send VT to obj
        }
        catch ( const out_of_range& ) {
        }
        catch ( const domain_error& ) {
        }
      }

      return;
    }
  }

  throw domain_error( "VT object not member of group" ); // Error: not group member
}

void VTDispatcher::Subscribe( stem::addr_type addr, oid_type oid, group_type grp )
{
  detail::vtime_obj_rec& r = vtmap[oid];
  r.addr = addr;
  r.add_group( grp );

  grmap.insert( make_pair(grp,oid) );
}

DEFINE_RESPONSE_TABLE( VTDispatcher )
  EV_Event_base_T_( ST_NULL, MESS, VTDispatch, VTmess )
END_RESPONSE_TABLE

char *Init_buf[128];
VTDispatcher *VTHandler::_vtdsp = 0;
static int *_rcount = 0;

void VTHandler::Init::__at_fork_prepare()
{
}

void VTHandler::Init::__at_fork_child()
{
  if ( *_rcount != 0 ) {
    VTHandler::_vtdsp->~VTDispatcher();
    VTHandler::_vtdsp = new( VTHandler::_vtdsp ) VTDispatcher();
  }
}

void VTHandler::Init::__at_fork_parent()
{
}

void VTHandler::Init::_guard( int direction )
{
  static xmt::recursive_mutex _init_lock;
  static int _count = 0;

  if ( direction ) {
    if ( _count++ == 0 ) {
#ifdef _PTHREADS
      _rcount = &_count;
      pthread_atfork( __at_fork_prepare, __at_fork_parent, __at_fork_child );
#endif
      VTHandler::_vtdsp = new VTDispatcher();
    }
  } else {
    --_count;
    if ( _count == 0 ) {
      delete VTHandler::_vtdsp;
      VTHandler::_vtdsp = 0;
    }
  }
}

VTHandler::Init::Init()
{ _guard( 1 ); }

VTHandler::Init::~Init()
{ _guard( 0 ); }

void VTHandler::VTSend( const stem::Event& ev )
{
}

VTHandler::VTHandler() :
   EventHandler()
{
  new( Init_buf ) Init();
}

VTHandler::VTHandler( const char *info ) :
   EventHandler( info )
{
  new( Init_buf ) Init();
}

VTHandler::VTHandler( stem::addr_type id, const char *info ) :
   EventHandler( id, info )
{
  new( Init_buf ) Init();
}

VTHandler::~VTHandler()
{
  ((Init *)Init_buf)->~Init();
}

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
    o << "  " << *i;
  }
  return o << " ]\n";
}

ostream& operator <<( ostream& o, const vt::vtime& v )
{ return o << v.vt; }

ostream& operator <<( ostream& o, const vt::gvtime_type::value_type& v )
{
  o << "group " << v.first << ": " << v.second.vt;
}

ostream& operator <<( ostream& o, const vt::gvtime_type& v )
{
  o << "{\n";
  for ( vt::gvtime_type::const_iterator i = v.begin(); i != v.end(); ++i ) {
    o << " " << *i;
  }
  return o << "}\n";
}

} // namespace std


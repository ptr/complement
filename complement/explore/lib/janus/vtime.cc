// -*- C++ -*- Time-stamp: <07/08/17 22:28:55 ptr>

#include <janus/vtime.h>

#include <iostream>
#include <stdint.h>
#include <stem/EvManager.h>

namespace janus {

using namespace std;
using namespace xmt;
using namespace stem;

void vtime::pack( std::ostream& s ) const
{
  __pack( s, static_cast<uint8_t>(vt.size()) );
  for ( vtime_type::const_iterator i = vt.begin(); i != vt.end(); ++i ) {
    i->first.pack( s ); // __pack( s, i->first );
    __pack( s, i->second );
  }
}

void vtime::net_pack( std::ostream& s ) const
{
  __net_pack( s, static_cast<uint8_t>(vt.size()) );
  for ( vtime_type::const_iterator i = vt.begin(); i != vt.end(); ++i ) {
    i->first.net_pack( s ); // __net_pack( s, i->first );
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

    oid.unpack( s ); // __unpack( s, oid );
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

    oid.net_unpack( s ); // __net_unpack( s, oid );
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

void VSsync_rq::pack( std::ostream& s ) const
{
  __pack( s, grp );
  __pack( s, mess );
}

void VSsync_rq::net_pack( std::ostream& s ) const
{
  __net_pack( s, grp );
  __net_pack( s, mess );
}

void VSsync_rq::unpack( std::istream& s )
{
  __unpack( s, grp );
  __unpack( s, mess );
}

void VSsync_rq::net_unpack( std::istream& s )
{
  __net_unpack( s, grp );
  __net_unpack( s, mess );
}

void VSsync::pack( std::ostream& s ) const
{
  gvt.pack( s );
  VSsync_rq::pack( s );
}

void VSsync::net_pack( std::ostream& s ) const
{
  gvt.net_pack( s );
  VSsync_rq::net_pack( s );
}

void VSsync::unpack( std::istream& s )
{
  gvt.unpack( s );
  VSsync_rq::unpack( s );
}

void VSsync::net_unpack( std::istream& s )
{
  gvt.net_unpack( s );
  VSsync_rq::net_unpack( s );
}

void VSmess::pack( std::ostream& s ) const
{
  __pack( s, code );
  src.pack( s ); // __pack( s, src );
  VSsync::pack( s );
}

void VSmess::net_pack( std::ostream& s ) const
{
  __net_pack( s, code );
  src.net_pack( s ); // __net_pack( s, src );
  VSsync::net_pack( s );
}

void VSmess::unpack( std::istream& s )
{
  __unpack( s, code );
  src.unpack( s ); // __unpack( s, src );
  VSsync::unpack( s );
}

void VSmess::net_unpack( std::istream& s )
{
  __net_unpack( s, code );
  src.net_unpack( s ); // __net_unpack( s, src );
  VSsync::net_unpack( s );
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

bool vtime_obj_rec::deliver( const VSmess& m )
{
  if ( order_correct( m ) ) {
    lvt[m.src] += m.gvt.gvt;
    lvt[m.src][m.grp][m.src] = vt.gvt[m.grp][m.src] + 1;
    sup( vt.gvt[m.grp], lvt[m.src][m.grp] );
    return true;
  }

  return false;
}

bool vtime_obj_rec::deliver_delayed( const VSmess& m )
{
  if ( order_correct_delayed( m ) ) {
    lvt[m.src] += m.gvt.gvt;
    lvt[m.src][m.grp][m.src] = vt.gvt[m.grp][m.src] + 1;
    sup( vt.gvt[m.grp], lvt[m.src][m.grp] );
    return true;
  }

  return false;
}

bool vtime_obj_rec::order_correct( const VSmess& m )
{
  if ( groups.find( m.grp ) == groups.end() ) {
    throw domain_error( "virtual synchrony object not member of group" );
  }

  gvtime gvt( m.gvt );

  if ( (vt.gvt[m.grp][m.src] + 1) != gvt[m.grp][m.src] ) {
    if ( (vt.gvt[m.grp][m.src] + 1) > gvt[m.grp][m.src] ) {
      throw out_of_range( "duplicate or wrong virtual synchrony message" );
    }
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

ostream& vtime_obj_rec::trace_deliver( const VSmess& m, ostream& o )
{
  if ( groups.find( m.grp ) == groups.end() ) {
    return o << "virtual synchrony object not member of group";
  }

  gvtime gvt( m.gvt );

  if ( (vt.gvt[m.grp][m.src] + 1) != gvt[m.grp][m.src] ) {
    if ( (vt.gvt[m.grp][m.src] + 1) > gvt[m.grp][m.src] ) {
      return o << "duplicate or wrong VT message, " << vt.gvt << " vs " << gvt;
    }
    return o << "counter violation, " << vt.gvt << " vs " << gvt;
  }

  vtime xvt = lvt[m.src][m.grp] + gvt[m.grp];
  xvt[m.src] = 0; // force exclude message originator, it checked above

  if ( !(xvt <= vt[m.grp]) ) {
    return o << "casuality violation, " << xvt << " vs " << vt[m.grp];
  }

  // check side casuality (via groups other then message's group)
  for ( groups_container_type::const_iterator l = groups.begin(); l != groups.end(); ++l ) {
    if ( (*l != m.grp) && !((lvt[m.src][*l] + gvt[*l]) <= vt[*l]) ) {
      return o << "side casuality violation, " << (lvt[m.src][*l] + gvt[*l]) << " vs " << vt[*l];
    }
  }

  return o << "should be delivered";
}

bool vtime_obj_rec::order_correct_delayed( const VSmess& m )
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

void vtime_obj_rec::delta( gvtime& vtstamp, const oid_type& from, const oid_type& to, group_type grp )
{
  vtstamp.gvt = vt.gvt - svt[to]; // delta
  vtstamp[grp][from] = vt.gvt[grp][from]; // my counter, as is, not delta
}

bool vtime_obj_rec::rm_group( group_type g )
{
  // strike out group g from my groups list
  groups_container_type::iterator i = groups.find( g );
  if ( i == groups.end() ) {
    throw domain_error( "virtual synchrony object not member of group" );
  }
  groups.erase( i );

  // remove my VT component for group g
  gvtime_type::iterator j = vt.gvt.find( g );

  if ( j != vt.gvt.end() ) {
    vt.gvt.erase( j );
  }

  // remove sended VT components for group g
  for ( snd_delta_vtime_t::iterator k = svt.begin(); k != svt.end(); ++k ) {
    gvtime_type::iterator l = k->second.find( g );
    if ( l != k->second.end() ) {
      k->second.erase( l );
    }
  }

  // remove recieved VT components for group g
  for ( delta_vtime_type::iterator k = lvt.begin(); k != lvt.end(); ++k ) {
    gvtime_type::iterator l = k->second.find( g );
    if ( l != k->second.end() ) {
      k->second.erase( l );
    }
  }

  // remove messages for group g that wait in delay pool
  for ( dpool_t::iterator p = dpool.begin(); p != dpool.end(); ) {
    if ( p->second->value().grp == g ) {
      dpool.erase( p++ );
    } else {
      ++p;
    }
  }

  return groups.empty() ? true : false;
}

void vtime_obj_rec::rm_member( const oid_type& oid )
{
  delta_vtime_type::iterator i = lvt.find( oid );

  if ( i != lvt.end() ) {
    lvt.erase( i );
  }

  snd_delta_vtime_t::iterator j = svt.find( oid );

  if ( j != lvt.end() ) {
    svt.erase( j );
  }
}

void vtime_obj_rec::sync( group_type g, const oid_type& oid, const gvtime_type& gvt )
{
  lvt[oid] = gvt;
  gvtime_type::const_iterator i = gvt.find( g );
  if ( i != gvt.end() ) {
    sup( vt.gvt[g], i->second.vt );
    // vtime_type::const_iterator j = i->second.vt.find( oid );
    // if ( j != i->second.vt.end() ) {
    //   vt.gvt[g][oid] = j->second;
    //   cerr << "**** " << gvt << endl;
    // }
  }
}

} // namespace detail

void Janus::settrf( unsigned f )
{
  scoped_lock _x1( _lock_tr );
  _trflags |= f;
}

void Janus::unsettrf( unsigned f )
{
  scoped_lock _x1( _lock_tr );
  _trflags &= (0xffffffff & ~f);
}

void Janus::resettrf( unsigned f )
{
  scoped_lock _x1( _lock_tr );
  _trflags = f;
}

void Janus::cleantrf()
{
  scoped_lock _x1( _lock_tr );
  _trflags = 0;
}

unsigned Janus::trflags() const
{
  scoped_lock _x1( _lock_tr );

  return _trflags;
}

void Janus::settrs( std::ostream *s )
{
  scoped_lock _x1( _lock_tr );
  _trs = s;
}

void Janus::JaDispatch( const stem::Event_base<VSmess>& m )
{
  pair<gid_map_type::const_iterator,gid_map_type::const_iterator> range =
    grmap.equal_range( m.value().grp );

  for ( ; range.first != range.second; ++range.first ) {
    vt_map_type::iterator i = vtmap.find( range.first->second );
    if ( i == vtmap.end() || i->second.stem_addr() == m.src() ) { // not for nobody and not for self
      continue;
    }
    try {
      // check local or remote? i->second.addr
      // if remote, forward it to foreign VTDispatcher?
      // looks, like local source shouldn't be here?
      check_and_send( i->second, m );
    }
    catch ( const out_of_range& err ) {
      try {
        scoped_lock lk(_lock_tr);
        if ( _trs != 0 && _trs->good() && (_trflags & tracefault) ) {
          *_trs << err.what() << " "
                << __FILE__ << ":" << __LINE__ << endl;
        }
      }
      catch ( ... ) {
      }
    }
    catch ( const domain_error& err ) {
      try {
        scoped_lock lk(_lock_tr);
        if ( _trs != 0 && _trs->good() && (_trflags & tracefault) ) {
          *_trs << err.what() << " "
                << __FILE__ << ":" << __LINE__ << endl;
        }
      }
      catch ( ... ) {
      }
    }
  }
}

void Janus::check_and_send( detail::vtime_obj_rec& vt, const stem::Event_base<VSmess>& m )
{
  if ( vt.deliver( m.value() ) ) {
    stem::Event ev( m.value().code );
    ev.dest(vt.stem_addr());
    ev.src(m.src());
    ev.value() = m.value().mess;
#ifdef __FIT_VS_TRACE
    try {
      scoped_lock lk(_lock_tr);
      if ( _trs != 0 && _trs->good() && (_trflags & tracedispatch) ) {
        *_trs << "Deliver " << m.value() << endl;
      }
    }
    catch ( ... ) {
    }
#endif // __FIT_VS_TRACE
    Forward( ev );
    check_and_send_delayed( vt );
  } else {
#ifdef __FIT_VS_TRACE
    try {
      scoped_lock lk(_lock_tr);
      if ( _trs != 0 && _trs->good() && (_trflags & tracedelayed) ) {
        *_trs << "Delayed " << m.value() << endl;
      }
    }
    catch ( ... ) {
    }
#endif // __FIT_VS_TRACE
    vt.dpool.push_back( make_pair( xmt::timespec(xmt::timespec::now), new Event_base<VSmess>(m) ) ); // 0 should be timestamp
  }
}

void Janus::check_and_send_delayed( detail::vtime_obj_rec& vt )
{
  typedef detail::vtime_obj_rec::dpool_t dpool_t;
  bool more;
  do {
    more = false;
    for ( dpool_t::iterator j = vt.dpool.begin(); j != vt.dpool.end(); ) {
      if ( vt.deliver_delayed( j->second->value() ) ) {
        stem::Event evd( j->second->value().code );
        evd.dest(vt.stem_addr());
        evd.src(j->second->src());
        evd.value() = j->second->value().mess;
#ifdef __FIT_VS_TRACE
        try {
          scoped_lock lk(_lock_tr);
          if ( _trs != 0 && _trs->good() && (_trflags & tracedispatch) ) {
            *_trs << "Deliver delayed " << j->second->value() << endl;
          }
        }
        catch ( ... ) {
        }
#endif // __FIT_VS_TRACE
        Forward( evd );
        delete j->second;
        vt.dpool.erase( j++ );
        more = true;
      } else {
#ifdef __FIT_VS_TRACE
        try {
          scoped_lock lk(_lock_tr);
          if ( _trs != 0 && _trs->good() && (_trflags & tracedispatch) ) {
            *_trs << "Remain delayed " << j->second->value()
                  << "\nReason: ";
            vt.trace_deliver( j->second->value(), *_trs ) << endl;
          }
        }
        catch ( ... ) {
        }
#endif // __FIT_VS_TRACE
        ++j;
      }
    }
  } while ( more );
}

void Janus::JaSend( const stem::Event& e, group_type grp )
{
  // This method not called from Dispatch, but work on the same level and in the same
  // scope, so this lock (from stem::EventHandler) required here:
  xmt::recursive_scoped_lock lk( this->_theHistory_lock );

  const pair<gid_map_type::const_iterator,gid_map_type::const_iterator> range =
    grmap.equal_range( grp );

  for ( gid_map_type::const_iterator o = range.first; o != range.second; ++o ) {
    vt_map_type::iterator i = vtmap.find( o->second );
    if ( i != vtmap.end() && i->second.stem_addr() == e.src() ) { // for self
      detail::vtime_obj_rec& vt = i->second;
      const oid_type& from = o->second;
      stem::Event_base<VSmess> m( VS_MESS );
      m.value().src = from; // oid
      m.value().code = e.code();
      m.value().mess = e.value();
      m.value().grp = grp;
      // m.dest( ??? ); // local VT dispatcher?
      m.src( e.src() );

      // This is like VTDispatch, but VT stamp in every message different,
      // in accordance with individual knowlage about object's VT.

      vt.next( from, grp ); // my counter

      for ( gid_map_type::const_iterator g = range.first; g != range.second; ++g ) {
        vt_map_type::iterator k = vtmap.find( g->second );
        if ( k == vtmap.end() || k->second.stem_addr() == m.src() ) { // not for nobody and not for self
          continue;
        }
        try {
          vt.delta( m.value().gvt, from, g->second, grp );

          // check local or remote? i->second.addr
          // if remote, forward it to foreign VTDispatcher?
          try {
            /* const transport tr = */ manager()->transport( k->second.stem_addr() );
            gaddr_type ga = manager()->reflect( k->second.stem_addr() );
            if ( ga != gaddr_type() ) {
              ga.addr = 2; // vtd
              addr_type a = manager()->reflect( ga );
              if ( a == badaddr ) {
                a = manager()->SubscribeRemote( ga, "vtd" );
              }
              m.dest( a );
              Forward( m );
              vt.base_advance(g->second); // store last send VT to obj
            }
          }
          catch ( const range_error& ) {
            check_and_send( k->second, m );
            vt.base_advance(g->second); // store last send VT to obj
          }
        }
        catch ( const out_of_range& err ) {
          try {
            scoped_lock lk(_lock_tr);
            if ( _trs != 0 && _trs->good() && (_trflags & tracefault) ) {
              *_trs << err.what() << " "
                    << __FILE__ << ":" << __LINE__ << endl;
            }
          }
          catch ( ... ) {
          }
        }
        catch ( const domain_error& err ) {
          try {
            scoped_lock lk(_lock_tr);
            if ( _trs != 0 && _trs->good() && (_trflags & tracefault) ) {
              *_trs << err.what() << " "
                    << __FILE__ << ":" << __LINE__ << endl;
            }
          }
          catch ( ... ) {
          }
        }
      }

      return;
    }
  }

  throw domain_error( "VT object not member of group" ); // Error: not group member
}

void Janus::Subscribe( stem::addr_type addr, oid_type oid, group_type grp )
{
  // See comment on top of VTSend above
  xmt::recursive_scoped_lock lk( this->_theHistory_lock );

  pair<gid_map_type::const_iterator,gid_map_type::const_iterator> range =
    grmap.equal_range( grp );

  for ( ; range.first != range.second; ++range.first ) {
    vt_map_type::iterator i = vtmap.find( range.first->second );
    if ( i != vtmap.end() ) {
      stem::Event_base<VSsync_rq> ev( VS_NEW_MEMBER );
      ev.dest( i->second.stem_addr() );
      ev.src( addr );
      ev.value().grp = grp;
#ifdef __FIT_VS_TRACE
      try {
        scoped_lock lk(_lock_tr);
        if ( _trs != 0 && _trs->good() && (_trflags & tracegroup) ) {
          *_trs << "VS_NEW_MEMBER " << ev.src() << " -> " << ev.dest() << endl;
        }
      }
      catch ( ... ) {
      }
#endif // __FIT_VS_TRACE
      Forward( ev );
    }
  }

  vtmap[oid].add( addr, grp );
  grmap.insert( make_pair(grp,oid) );
}

void Janus::Unsubscribe( oid_type oid, group_type grp )
{
  // See comment on top of VTSend above
  xmt::recursive_scoped_lock lk( this->_theHistory_lock );

  pair<gid_map_type::iterator,gid_map_type::iterator> range =
    grmap.equal_range( grp );

  vt_map_type::iterator i = vtmap.find( oid );
  while ( range.first != range.second ) {
    if ( range.first->second == oid ) {
      grmap.erase( range.first++ );
    } else {
      vt_map_type::iterator j = vtmap.find( range.first->second );
      if ( j != vtmap.end() ) {
        stem::Event_base<VSsync_rq> ev( VS_OUT_MEMBER );
        ev.dest( j->second.stem_addr() );
        ev.src( i != vtmap.end() ? i->second.stem_addr() : self_id() );
        ev.value().grp = grp;
#ifdef __FIT_VS_TRACE
        try {
          scoped_lock lk(_lock_tr);
          if ( _trs != 0 && _trs->good() && (_trflags & tracegroup) ) {
            *_trs << "VS_OUT_MEMBER " << ev.src() << " -> " << ev.dest() << endl;
          }
        }
        catch ( ... ) {
        }
#endif // __FIT_VS_TRACE

        Forward( ev );
      }
      ++range.first;
    }
  }

  if ( i != vtmap.end() ) {
    if ( i->second.rm_group( grp ) ) { // no groups more
      vtmap.erase( i );
    }
  }
}

void Janus::Unsubscribe( oid_type oid )
{
  // See comment on top of JaSend above
  xmt::recursive_scoped_lock lk( this->_theHistory_lock );

  vt_map_type::iterator i = vtmap.find( oid );
  if ( i != vtmap.end() ) {
    list<group_type> grp_list;
    i->second.groups_list( back_inserter( grp_list ) );
    for ( list<group_type>::const_iterator grp = grp_list.begin(); grp != grp_list.end(); ++grp ) {

      pair<gid_map_type::iterator,gid_map_type::iterator> range =
        grmap.equal_range( *grp );

      while ( range.first != range.second ) {
        if ( range.first->second == oid ) {
          grmap.erase( range.first++ );
        } else {
          vt_map_type::iterator j = vtmap.find( range.first->second );
          if ( j != vtmap.end() ) {
            stem::Event_base<VSsync_rq> ev( VS_OUT_MEMBER );
            ev.dest( j->second.stem_addr() );
            ev.src( i != vtmap.end() ? i->second.stem_addr() : self_id() );
            ev.value().grp = *grp;
#ifdef __FIT_VS_TRACE
            try {
              scoped_lock lk(_lock_tr);
              if ( _trs != 0 && _trs->good() && (_trflags & tracegroup) ) {
                *_trs << "VS_OUT_MEMBER " << ev.src() << " -> " << ev.dest() << endl;
              }
            }
            catch ( ... ) {
            }
#endif // __FIT_VS_TRACE

            Forward( ev );
          }
          ++range.first;
        }
      }
      i->second.rm_group( *grp );
    }
    vtmap.erase( i );
  }
}

void Janus::get_gvtime( group_type grp, stem::addr_type addr, gvtime_type& gvt )
{
  // See comment on top of JaSend above
  xmt::recursive_scoped_lock lk( this->_theHistory_lock );

  pair<gid_map_type::iterator,gid_map_type::iterator> range =
    grmap.equal_range( grp );
  for ( ; range.first != range.second; ++range.first ) {
    vt_map_type::iterator i = vtmap.find( range.first->second );
    if ( i != vtmap.end() && i->second.stem_addr() == addr ) {
      i->second.get_gvt( gvt );
      return;
    }
  }

  try {
    scoped_lock lk(_lock_tr);
    if ( _trs != 0 && _trs->good() && (_trflags & tracefault) ) {
      *_trs << "virtual synchrony object not member of group" << " " << __FILE__ << ":" << __LINE__ << endl;
    }
  }
  catch ( ... ) {
  }

  throw domain_error( "virtual synchrony object not member of group" ); // Error: not group member
}

void Janus::set_gvtime( group_type grp, stem::addr_type addr, const gvtime_type& gvt )
{
  // See comment on top of JaSend above
  xmt::recursive_scoped_lock lk( this->_theHistory_lock );

  pair<gid_map_type::iterator,gid_map_type::iterator> range =
    grmap.equal_range( grp );
  for ( ; range.first != range.second; ++range.first ) {
    vt_map_type::iterator i = vtmap.find( range.first->second );
    if ( i != vtmap.end() && i->second.stem_addr() == addr ) {
#ifdef __FIT_VS_TRACE
      try {
        scoped_lock lk(_lock_tr);
        if ( _trs != 0 && _trs->good() && (_trflags & tracegroup) ) {
          *_trs << "Set gvt G" << grp << " " << i->first
                << " (" << addr << ") " << gvt << endl;
        }
      }
      catch ( ... ) {
      }
#endif // __FIT_VS_TRACE
      i->second.sync( grp, i->first, gvt );
      check_and_send_delayed( i->second );
      return;
    }
  }

  try {
    scoped_lock lk(_lock_tr);
    if ( _trs != 0 && _trs->good() && (_trflags & tracefault) ) {
      *_trs << "virtual synchrony object not member of group" << " " << __FILE__ << ":" << __LINE__ << endl;
    }
  }
  catch ( ... ) {
  }

  throw domain_error( "virtual synchrony object not member of group" ); // Error: not group member
}

DEFINE_RESPONSE_TABLE( Janus )
  EV_Event_base_T_( ST_NULL, VS_MESS, JaDispatch, VSmess )
END_RESPONSE_TABLE

char *Init_buf[128];
Janus *VTHandler::_vtdsp = 0;
static int *_rcount = 0;

void VTHandler::Init::__at_fork_prepare()
{
}

void VTHandler::Init::__at_fork_child()
{
  // VTDispatcher not start threads (at least yet), so following not required:
  // if ( *_rcount != 0 ) {
  //   VTHandler::_vtdsp->~VTDispatcher();
  //   VTHandler::_vtdsp = new( VTHandler::_vtdsp ) VTDispatcher();
  // }
}

void VTHandler::Init::__at_fork_parent()
{
}

void VTHandler::Init::_guard( int direction )
{
  static xmt::recursive_mutex _init_lock;

  xmt::recursive_scoped_lock lk(_init_lock);
  static int _count = 0;

  if ( direction ) {
    if ( _count++ == 0 ) {
#ifdef _PTHREADS
      _rcount = &_count;
      pthread_atfork( __at_fork_prepare, __at_fork_parent, __at_fork_child );
#endif
      VTHandler::_vtdsp = new Janus( janus_addr, "janus" );
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

void VTHandler::JaSend( const stem::Event& ev )
{
  ev.src( self_id() );
  _vtdsp->JaSend( ev, ev.dest() ); // throw domain_error, if not group member
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
  _vtdsp->Unsubscribe( oid_type( self_id() ) );

  ((Init *)Init_buf)->~Init();
}

void VTHandler::VSNewMember( const stem::Event_base<VSsync_rq>& ev )
{
  stem::Event_base<VSsync> out_ev( VS_SYNC_TIME );
  out_ev.dest( ev.src() );
  out_ev.value().grp = ev.value().grp;
  get_gvtime( ev.value().grp, out_ev.value().gvt.gvt );

  Send( out_ev );
}

void VTHandler::VSNewMember_data( const stem::Event_base<VSsync_rq>& ev, const string& data )
{
  stem::Event_base<VSsync> out_ev( VS_SYNC_TIME );
  out_ev.dest( ev.src() );
  out_ev.value().grp = ev.value().grp;
  get_gvtime( ev.value().grp, out_ev.value().gvt.gvt );
  out_ev.value().mess = data;

  Send( out_ev );
}

void VTHandler::VSOutMember( const stem::Event_base<VSsync_rq>& )
{
}

void VTHandler::VSsync_time( const stem::Event_base<VSsync>& ev )
{
  try {
    // sync data from ev.value().mess
    _vtdsp->set_gvtime( ev.value().grp, self_id(), ev.value().gvt.gvt );
  }
  catch ( const domain_error&  ) {
  }
}

DEFINE_RESPONSE_TABLE( VTHandler )
  EV_Event_base_T_( ST_NULL, VS_NEW_MEMBER, VSNewMember, VSsync_rq )
  EV_Event_base_T_( ST_NULL, VS_OUT_MEMBER, VSOutMember, VSsync_rq )
  EV_Event_base_T_( ST_NULL, VS_SYNC_TIME, VSsync_time, VSsync )
END_RESPONSE_TABLE

} // namespace vt

namespace std {

ostream& operator <<( ostream& o, const janus::vtime_type::value_type& v )
{
  return o << "(" << v.first << "," << v.second << ")";
}

ostream& operator <<( ostream& o, const janus::vtime_type& v )
{
  for ( janus::vtime_type::const_iterator i = v.begin(); i != v.end(); ++i ) {
    if ( i != v.begin() ) {
      o << ", ";
    }
    o << *i;
  }
  return o;
}

ostream& operator <<( ostream& o, const janus::vtime& v )
{ return o << v.vt; }

ostream& operator <<( ostream& o, const janus::gvtime_type::value_type& v )
{
  o << v.first << ": " << v.second.vt;
}

ostream& operator <<( ostream& o, const janus::gvtime_type& v )
{
  o << "{\n";
  for ( janus::gvtime_type::const_iterator i = v.begin(); i != v.end(); ++i ) {
    o << "\t" << *i << "\n";
  }
  return o << "}\n";
}

ostream& operator <<( ostream& o, const janus::gvtime& v )
{
  return o << v.gvt;
}

ostream& operator <<( ostream& o, const janus::VSsync& m )
{
  // ios_base::fmtflags f = o.flags( ios_base::hex );
  o << "G" << m.grp << " " << m.gvt;
  
  return o;
}

ostream& operator <<( ostream& o, const janus::VSmess& m )
{
  ios_base::fmtflags f = o.flags( ios_base::hex | ios_base::showbase );
  o << "C" << m.code << dec << " " << m.src << " " << static_cast<const janus::VSsync&>(m);
  o.flags( f );
  return o;
}

} // namespace janus


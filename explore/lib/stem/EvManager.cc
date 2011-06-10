// -*- C++ -*- Time-stamp: <2011-06-10 15:42:36 yeti>

/*
 *
 * Copyright (c) 1995-1999, 2002, 2003, 2005-2010
 * Petr Ovtchenkov
 *
 * Copyright (c) 1999-2001
 * ParallelGraphics Ltd.
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include <config/feature.h>
#include "stem/EvManager.h"
#include "stem/NetTransport.h"
#include <iomanip>
#include <mt/mutex>

#include <exam/defs.h>
#include <mt/callstack.h>

namespace stem {

using namespace std;
using namespace std::tr2;

const addr_type& badaddr = xmt::nil_uuid;
const code_type badcode  = 0xffffffff;

std::string EvManager::inv_key_str( "invalid key" );

static const string addr_unknown("address unknown");
static const string no_catcher( "no catcher for event" );

unsigned int EvManager::working_threads = 2;

EvManager::EvManager() :
    _dispatch_stop( true ),
    _trflags( 0 ),
    _trs( 0 ),
    n_threads( working_threads ),
    workers( n_threads )
{
  vertices[EventHandler::domain()];
}

EvManager::~EvManager()
{
#if 0
  if ( !_dispatch_stop ) {
    // never be here:
    // worker loops should be stopped before EvManager's dtor call!
    cerr << HERE << endl;
    _dispatch_stop = true;

    for ( unsigned int i = 0; i < n_threads; ++i ) {
      cerr << HERE << endl;
      {
        std::tr2::unique_lock<std::tr2::mutex> lock( workers[i]->lock );
        workers[i]->cnd.notify_one();
        cerr << HERE << endl;
      }
      delete workers[i];
      cerr << HERE << endl;
    }
  }
#endif
}

/* Oversimplified start_queue, review later */
void EvManager::start_queue()
{
  _dispatch_stop = false;
  for ( unsigned int i = 0; i < n_threads; ++i ) {
    workers[i] = new worker( this );
  }
}

/* Oversimplified stop_queue, review later */
void EvManager::stop_queue()
{
  if ( _dispatch_stop ) {
    return;
  }
   
  _dispatch_stop = true;

  for ( unsigned int i = 0; i < n_threads; ++i ) {
    {
      std::tr2::unique_lock<std::tr2::mutex> lock( workers[i]->lock );
      workers[i]->cnd.notify_one();
    }
    delete workers[i];
  }
}

void EvManager::push( const Event& e )
{
  unsigned int i = e.dest().u.i[0] & (n_threads - 1);
  std::tr2::lock_guard<std::tr2::mutex> lock( workers[i]->lock );
  workers[i]->events.push_back( e );
  workers[i]->cnd.notify_one();
}

void EvManager::worker::_loop( worker* p )
{
  worker& me = *p;
  EventHandler* obj;

  for ( ; ; ) {
    try {
      list< Event > events;

      {
        std::tr2::unique_lock<std::tr2::mutex> lock( me.lock );
        me.cnd.wait( lock, me.not_empty );
        swap( me.events, events );
      }

      for ( list< stem::Event >::const_iterator i = events.begin();i != events.end();++i ) {
        const Event& ev = *i;
        {
          std::tr2::lock_guard<std::tr2::rw_mutex> lock( me.mgr->_lock_heap );
          local_heap_type::iterator k = me.mgr->heap.find( ev.dest() );

          if ( k == me.mgr->heap.end() ) {
            continue;
          }

          if ( me.mgr->is_domain( k->second.domain ) ) { // is it domain indeed?
            std::tr2::basic_read_lock<std::tr2::rw_mutex> elock( me.mgr->_lock_edges );
            std::tr2::basic_read_lock<std::tr2::rw_mutex> glock( me.mgr->_lock_gate );

            auto eid = me.mgr->gate.find( k->second.domain );
            if ( eid != me.mgr->gate.end() ) {
              auto bid = me.mgr->bridges.find( eid->second );
              if ( bid != me.mgr->bridges.end() ) {
                reinterpret_cast<NetTransport_base*>(bid->second)->Dispatch( ev );
              }
            }
            continue;
          }

          obj = reinterpret_cast<EventHandler*>(k->second.object);

          obj->_theHistory_lock.lock();
        }

        obj->Dispatch( ev );
        obj->_theHistory_lock.unlock();
      }

      if ( me.mgr->_dispatch_stop) {
        break;
      }
    }
    catch (...) {
      if ( obj != 0 ) {
        obj->_theHistory_lock.try_lock();
        obj->_theHistory_lock.unlock();
      }
    }
  }
}

void EvManager::Unsubscribe( const addr_type& id )
{
  {
    std::tr2::lock_guard<std::tr2::rw_mutex> _x1( _lock_heap );
    std::tr2::lock_guard<std::tr2::mutex> lk( _lock_iheap );

    unsafe_Unsubscribe( id );
  }
}

void EvManager::unsafe_Subscribe( const addr_type& id, EventHandler* object )
{
#ifdef __FIT_STEM_TRACE
  try {
    lock_guard<mutex> lk(_lock_tr);
    if ( _trs != 0 && _trs->good() && (_trflags & tracesubscr) ) {
      ios_base::fmtflags f = _trs->flags( ios_base::showbase );
      *_trs << "EvManager subscribe " << id << ' '
            << object << " ("
            << xmt::demangle( object->classtype().name() ) << ")" << endl;
#ifdef STLPORT
      _trs->flags( f );
#else
      _trs->flags( static_cast<std::_Ios_Fmtflags>(f) );
#endif
    }
  }
  catch ( ... ) {
  }
#endif // __FIT_STEM_TRACE

  heap[id].object = object;
}

void EvManager::unsafe_Subscribe( const addr_type& id, const domain_type& d )
{
#ifdef __FIT_STEM_TRACE
  try {
    lock_guard<mutex> lk(_lock_tr);
    if ( _trs != 0 && _trs->good() && (_trflags & tracesubscr) ) {
      ios_base::fmtflags f = _trs->flags( ios_base::showbase );
      *_trs << "EvManager subscribe " << id << ' ' << d << endl;
#ifdef STLPORT
      _trs->flags( f );
#else
      _trs->flags( static_cast<std::_Ios_Fmtflags>(f) );
#endif
    }
  }
  catch ( ... ) {
  }
#endif // __FIT_STEM_TRACE

  heap[id].domain = d;
}

void EvManager::unsafe_annotate( const addr_type& id, const std::string& info )
{
  info_heap_type::iterator i = iheap.find( info );
  if ( i == iheap.end() ) {
    iheap[info].push_back( id );
  } else {
    for ( addr_collection_type::const_iterator j = i->second.begin(); j != i->second.end(); ++j ) {
      if ( *j == id ) {
        return;
      }
    }
    i->second.push_back( id );
  }
}

void EvManager::unsafe_Unsubscribe( const addr_type& id )
{
  auto k = heap.find( id );

  if ( k == heap.end() ) {
    return;
  }

#ifdef __FIT_STEM_TRACE
  try {
    lock_guard<mutex> lk(_lock_tr);
    if ( _trs != 0 && _trs->good() && (_trflags & tracesubscr) ) {
      ios_base::fmtflags f = _trs->flags( ios_base::showbase );
      *_trs << "EvManager unsubscribe " << id << ' ';

      if ( is_domain( k->second.domain ) ) {
        *_trs << k->second.domain;
      } else {
        const EventHandler* obj = reinterpret_cast<const EventHandler*>(k->second.object);
        *_trs << obj << " ("
              << xmt::demangle( obj->classtype().name() ) << ')';
      }
      *_trs << endl;

#ifdef STLPORT
      _trs->flags( f );
#else
      _trs->flags( static_cast<std::_Ios_Fmtflags>(f) );
#endif
    }
  }
  catch ( ... ) {
  }
#endif // __FIT_STEM_TRACE

  heap.erase( k );

  list<info_heap_type::key_type> trash;
  for ( info_heap_type::iterator i = iheap.begin(); i != iheap.end(); ++i ) {
    for ( addr_collection_type::iterator j = i->second.begin(); j != i->second.end(); ++j ) {
      if ( *j == id ) {
        i->second.erase( j );
        break;
      }
    }

    if ( i->second.empty() ) {
      trash.push_back( i->first );
    }
  }
  for ( list<info_heap_type::key_type>::const_iterator i = trash.begin(); i != trash.end(); ++i ) {
    iheap.erase( *i );
  }
}

void EvManager::settrf( unsigned f )
{
  lock_guard<mutex> _x1( _lock_tr );
  _trflags |= f;
}

void EvManager::unsettrf( unsigned f )
{
  lock_guard<mutex> _x1( _lock_tr );
  _trflags &= (0xffffffff & ~f);
}

void EvManager::resettrf( unsigned f )
{
  lock_guard<mutex> _x1( _lock_tr );
  _trflags = f;
}

void EvManager::cleantrf()
{
  lock_guard<mutex> _x1( _lock_tr );
  _trflags = 0;
}

unsigned EvManager::trflags() const
{
  lock_guard<mutex> _x1( _lock_tr );
  return _trflags;
}

std::ostream* EvManager::settrs( std::ostream* s )
{
  lock_guard<mutex> _x1( _lock_tr );
  std::ostream* tmp = _trs;
  _trs = s;

  return tmp;
}

bool EvManager::unsafe_is_local( const addr_type& id ) const
{
  local_heap_type::const_iterator k = heap.find( id );

  if ( k == heap.end() ) {
    return false;
  }

  return !is_domain( k->second.domain );
}

bool EvManager::is_domain( const domain_type& d ) const
{
  int _ver = xmt::uid_version( d );
  int _var = xmt::uid_variant( d );

  return ((_ver > 2) && (_ver < 6) && (_var == 2));
}

std::ostream& EvManager::dump( std::ostream& s ) const
{
  ios_base::fmtflags f = s.flags( ios_base::showbase );

  {
    basic_read_lock<rw_mutex> lk( _lock_heap );

    for ( local_heap_type::const_iterator i = heap.begin(); i != heap.end(); ++i ) {
      s << i->first << " => ";

      if ( is_domain(i->second.domain) ) {
        // it domain indeed!
        s << i->second.domain;
      } else {
        s << i->second.object
          << " (" << xmt::demangle( reinterpret_cast<EventHandler*>(i->second.object)->classtype().name() ) << ")";
      }
      s << '\n';
    }
  }

  s << '\n';
  {
    lock_guard<mutex> lk( _lock_iheap );

    for ( info_heap_type::const_iterator i = iheap.begin(); i != iheap.end(); ++i ) {
      s << i->first << " => '";
      for ( addr_collection_type::const_iterator j = i->second.begin(); j != i->second.end(); ) {
        s << *j;
        if ( ++j != i->second.end() ) {
          s << ' ';
        }
      }
      s << "'\n";
    }
  }

  s << endl;
#ifdef STLPORT
  s.flags( f );
#else
  s.flags( static_cast<std::_Ios_Fmtflags>(f) );
#endif

  return s;
}

EvManager::edge_id_type EvManager::bridge( NetTransport_base* b, const domain_type& domain )
{
  edge_id_type ne = xmt::uid();

  std::tr2::lock_guard<std::tr2::rw_mutex> lock( _lock_edges );

  edges.insert( make_pair( ne, make_pair( make_pair(EventHandler::domain(),domain), 1000) ) );
  vertices[EventHandler::domain()].push_back( ne );
  bridges[ne] = b;

  return ne;
}

void EvManager::connectivity( const edge_id_type& eid, const domain_type& u, const domain_type& v, unsigned w, NetTransport_base* b )
{
  std::tr2::lock_guard<std::tr2::rw_mutex> lock( _lock_edges );

  if ( edges.insert( make_pair( eid, make_pair( make_pair(u,v), w) ) ).second ) {
    vertices[u].push_back( eid );
    vertices[v];
  } else {
    if ( u != EventHandler::domain() ) {
      edges[eid].second = w;
    }
  }

  if ( (b != 0) && (u == EventHandler::domain()) ) {
    bridges[eid] = b;
  }
}

void EvManager::route_calc()
{
  /*
    Special form of Dijkstra algorithm: find starting edge (gateway) for
    shortest path from source (this domain) to other accessible
    vertex (other domain) of directed weighted graph.

    In this algo may be more then one directed edge from
    vertex u to vertex v (may be with different weight).
    That's why used 'edge id', instead of predecessor vertex.
  */

  typedef std::unordered_map<domain_type,unsigned> d_type;

  struct q_less
  {
      d_type& _d;

      q_less( d_type& d ) :
          _d( d )
        { }

      bool operator ()(const domain_type& l, const domain_type& r )
        { return (_d[l] > _d[r]); }
  };

  typedef std::vector<domain_type> q_type;

  pi_type pi;
  d_type d; // destination weights
  q_type Q; // heap, d[Q[0]] is minimal in d

  basic_read_lock<rw_mutex> lk( _lock_edges );

  Q.reserve( vertices.size() );

  for ( auto i = vertices.begin(); i != vertices.end(); ++i ) {
    Q.push_back( i->first );
    d[i->first] = numeric_limits<unsigned>::max();
    pi[i->first] = xmt::nil_uuid;
  }

  d[EventHandler::domain()] = 0; // source

  make_heap( Q.begin(), Q.end(), q_less( d ) );

  q_type::iterator i = Q.end();

  unsigned path_w;
  unsigned w;
  vertex_container_type::iterator vi;
  edge_container_type::iterator ei;

  while ( i != Q.begin() ) { // while Q not empty...
    path_w = d[*Q.begin()]; // u = *Q.begin(); u has min weight in Q
    if ( path_w == numeric_limits<unsigned>::max() ) {
      break; // only unaccessible remains
    }
    vi = vertices.find( *Q.begin() );

    pop_heap( Q.begin(), i--, q_less( d ) ); // remove u from Q

    // iterate through edges (u,v)
    for ( auto j = vi->second.begin(); j != vi->second.end(); ++j ) {
      ei = edges.find( *j );
      w = path_w + ei->second.second; // w(path to u) + w(u,v)
      if ( w < d[ei->second.first.second] ) { // relax
        d[ei->second.first.second] = w;
        pi[ei->second.first.second] = *j;
        make_heap( Q.begin(), i, q_less( d ) );
      }
    }
  }

  for ( auto k = pi.begin(); k != pi.end(); ++k ) {
    if ( k->second != xmt::nil_uuid ) {
      auto j = pi.find( edges[k->second].first.first );
      while ( (j != pi.end()) && (j->second != xmt::nil_uuid) ) {
        k->second = j->second;
        j = pi.find( edges[j->second].first.first );
      }
    }
  }

  lock_guard<rw_mutex> glk( _lock_gate );
  pi.swap( gate );
}

} // namespace stem

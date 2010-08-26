// -*- C++ -*- Time-stamp: <10/07/30 14:23:52 ptr>

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

#ifdef _MSC_VER
#pragma warning( disable : 4804 )
#endif

#include <config/feature.h>
#include "stem/EvManager.h"
#include "stem/NetTransport.h"
#include <iomanip>
#include <mt/mutex>

// #include <typeinfo>
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

__FIT_DECLSPEC EvManager::EvManager() :
    _id( xmt::uid() ),
    _dispatch_stop( false ),
    _trflags( 0 ),
    _trs( 0 ),
    n_threads( 2 ), // must be power of 2
    workers( n_threads )
{
  for ( unsigned int i = 0; i < n_threads; ++i ) {
    workers[i] = new worker( this );
  }
}

__FIT_DECLSPEC EvManager::~EvManager()
{
  _dispatch_stop = true;

  for ( unsigned int i = 0; i < n_threads; ++i ) {
    {
      std::tr2::unique_lock<std::tr2::mutex> lock( workers[i]->lock );
      workers[i]->cnd.notify_one();
    }
    delete workers[i];
  }
}

__FIT_DECLSPEC void EvManager::push( const Event& e )
{
  // misc::use_syslog<LOG_DEBUG,LOG_USER>() << HERE << ':' << e.code() << endl;
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
          obj = (k != me.mgr->heap.end()) ? k->second.top().second.second : 0;
          if ( obj == 0 ) {
            // cerr << "unknown addr " << ev.dest() << endl; 
            continue;
          }

          obj->_theHistory_lock.lock();
        }

        obj->Dispatch( ev );
        obj->_theHistory_lock.unlock();
      }

      if ( me.mgr->_dispatch_stop) {
        break;
      }
    } catch(...) {
      if ( obj != 0 ) {
        obj->_theHistory_lock.try_lock();
        obj->_theHistory_lock.unlock();
      }
      cerr << HERE << ':' << "unexpected" << endl;
    }
  }
}

void EvManager::Unsubscribe( const addr_type& id, EventHandler* obj )
{
  {
    std::tr2::lock_guard<std::tr2::rw_mutex> _x1( _lock_heap );
    std::tr2::lock_guard<std::tr2::mutex> lk( _lock_iheap );

    unsafe_Unsubscribe( id, obj );
  }
}

void EvManager::unsafe_Subscribe( const addr_type& id, EventHandler* object, int nice )
{
#ifdef __FIT_STEM_TRACE
  try {
    lock_guard<mutex> lk(_lock_tr);
    if ( _trs != 0 && _trs->good() && (_trflags & tracesubscr) ) {
      ios_base::fmtflags f = _trs->flags( ios_base::showbase );
      *_trs << "EvManager subscribe " << id << " nice " << nice << ' '
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

  heap[id].push( make_pair( nice, make_pair(object->flags(),object)) );
}

void EvManager::unsafe_Unsubscribe( const addr_type& id, EventHandler* obj )
{
#ifdef __FIT_STEM_TRACE
  try {
    lock_guard<mutex> lk(_lock_tr);
    if ( _trs != 0 && _trs->good() && (_trflags & tracesubscr) ) {
      ios_base::fmtflags f = _trs->flags( ios_base::showbase );
      *_trs << "EvManager unsubscribe " << id << ' '
            << obj << " ("
            << xmt::demangle( obj->classtype().name() ) << ')' << endl;
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

  local_heap_type::iterator i = heap.find( id );
  if ( i == heap.end() ) {
    return;
  }
  if ( i->second.size() == 1 ) {
    if ( i->second.top().second.second == obj ) {
      heap.erase( i );
    } else {
      try {
        lock_guard<mutex> lk(_lock_tr);
        if ( _trs != 0 && _trs->good() && (_trflags & tracefault) ) {
          *_trs << "EvManager unsubscribe: address " << id << " not correspond to "
                << obj << " ("
                << xmt::demangle( obj->classtype().name() ) << ")\n";
        }
      }
      catch ( ... ) {
      }
      return;
    }
  } else {
    misc::use_syslog<LOG_DEBUG,LOG_USER>() << HERE << endl;
    handlers_type tmp;
    swap( i->second, tmp );
    while ( !tmp.empty() ) {
      weighted_handler_type handler = tmp.top();
      if ( handler.second.second != obj ) {
        i->second.push( handler );
      }
      tmp.pop();
    }
    // check case: nothing was returned back;
    // this may happens if one object had few addresses
    if ( i->second.empty() ) {
      heap.erase( i );
    }
  }
  
  list<info_heap_type::key_type> trash;
  for ( info_heap_type::iterator i = iheap.begin(); i != iheap.end(); ++i ) {
    for ( addr_collection_type::iterator j = i->second.begin(); j != i->second.end(); ++j ) {
      if ( *j == id ) {
        // basic_read_lock<rw_mutex> _x1( _lock_heap );

        if ( heap.find( id ) == heap.end() ) { // all objects with this addr removed
          i->second.erase( j );
        }
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

__FIT_DECLSPEC std::ostream& EvManager::dump( std::ostream& s ) const
{
  ios_base::fmtflags f = s.flags( ios_base::showbase );

  {
    basic_read_lock<rw_mutex> lk( _lock_heap );

    for ( local_heap_type::const_iterator i = heap.begin(); i != heap.end(); ++i ) {
      s << i->first << " => " << i->second.top().second.second
        << " (" << xmt::demangle( i->second.top().second.second->classtype().name() ) << ")\n";
    }
  }

  s << '\n'; // "\nInfo map:\n";
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

} // namespace stem

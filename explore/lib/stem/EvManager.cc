// -*- C++ -*- Time-stamp: <10/08/19 20:12:12 ptr>

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
#include "stem/EDSEv.h"
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
    not_empty( *this ),
    n_threads( 6 ),
    _dispatch_stop( false ),
    _trflags( 0 ),
    _trs( 0 )
{
  for ( int k = 0; k < n_threads; ++k ) {
    threads.push_back( new thread(_Dispatch_sub, this ) );
  }
}

__FIT_DECLSPEC EvManager::~EvManager()
{
  stop_queue();
}

void EvManager::stop_queue()
{
  {
    lock_guard<mutex> lk( pheap_lock );
    _dispatch_stop = true;
    _cnd_queue.notify_all();
    // _cnd_retry.notify_all();
  }

  for ( list<thread*>::iterator k = threads.begin(); k != threads.end(); ++k ) {
    (*k)->join();
    delete *k;
    *k = 0;
  }
}

void EvManager::start_queue()
{
}

void EvManager::_Dispatch_sub( EvManager* p )
{
  EvManager& me = *p;
  EventHandler* obj = 0;
  int n = 0;

  EventHandler* next = 0;

  for ( ; ; ) {
    // cerr << '*';
    unique_lock<mutex> plk(me.pheap_lock);
    me._cnd_queue.wait( plk, me.not_empty );

    if ( me._dispatch_stop ) {
      return;
    }

    p_heap_type::iterator i = me.pheap.find( next );

    if ( i == me.pheap.end() ) {
      if ( me.pheap.empty() ) {
        continue;
      }
      
      i = me.pheap.begin();
      next = i->first;
    }

    // if ( i->second.lock == 0 ) {
    //   abort();
    // }
    
    while ( !i->second.lock->try_lock() ) {
      // cerr << '.';
      ++i;
      if ( i == me.pheap.end() ) {
        i = me.pheap.begin();
      }
      if ( (i == me.pheap.end()) || (i->first == next) ) {
        // cerr << '~';
        goto repeate; // .... next iteration in outer loop
      }

      if ( i->second.lock == 0 ) {
        // cerr << '!';
        abort();
      }
    }

    {
      Event ev;

      swap( ev, i->second.evs.front() );
      i->second.evs.pop();
      obj = i->first;

      plk.unlock();

      try {
#ifdef __FIT_STEM_TRACE
        try {
          lock_guard<mutex> lk(me._lock_tr);
          if ( me._trs != 0 && me._trs->good() && (me._trflags & tracedispatch) ) {
            *me._trs << xmt::demangle( obj->classtype().name() )
                     << " (" << obj << ")\n";
            if ( (me._trflags & tracetime) ) {
              *me._trs << std::tr2::get_system_time().nanoseconds_since_epoch().count();
            }
            obj->DispatchTrace( ev, *me._trs );
            *me._trs << endl;
          }
        }
        catch ( ... ) {
        }
#endif // __FIT_STEM_TRACE
        if ( !obj->Dispatch( ev ) ) {
          throw std::logic_error( no_catcher );
        }
      }
      catch ( std::logic_error& err ) {
        try {
          lock_guard<mutex> lk(me._lock_tr);
          if ( me._trs != 0 && me._trs->good() && (me._trflags & tracefault) ) {
            *me._trs << err.what() << "\n"
                     << xmt::demangle( obj->classtype().name() ) << " (" << obj << ")\n";
            if ( (me._trflags & tracetime) ) {
              *me._trs << std::tr2::get_system_time().nanoseconds_since_epoch().count();
            }
            obj->DispatchTrace( ev, *me._trs );
            *me._trs << endl;
          }
        }
        catch ( ... ) {
        }
      }
      catch ( ... ) {
        try {
          lock_guard<mutex> lk(me._lock_tr);
          if ( me._trs != 0 && me._trs->good() && (me._trflags & tracefault) ) {
            *me._trs << "Unknown, uncatched exception during process:\n"
                     << xmt::demangle( obj->classtype().name() ) << " (" << obj << ")\n";
            if ( (me._trflags & tracetime) ) {
              *me._trs << std::tr2::get_system_time().nanoseconds_since_epoch().count();
            }
            obj->DispatchTrace( ev, *me._trs );
            *me._trs << endl;
          }
        }
        catch ( ... ) {
        }
      }
    }

    plk.lock();
    if ( i->second.evs.empty() ) {
      mutex* m = 0;
      swap( m, i->second.lock );
      me.pheap.erase( i++ );
      m->unlock();
      delete m;
    } else {
      (i++)->second.lock->unlock();
    }

    next = (i == me.pheap.end()) ? 0 : i->first;

    repeate:
     ;
    // me._cnd_retry.notify_one();
  }
}

void EvManager::Unsubscribe( const addr_type& id, EventHandler* obj )
{
  {
    std::tr2::lock_guard<std::tr2::rw_mutex> _x1( _lock_heap );
    std::tr2::lock_guard<std::tr2::mutex> lk( _lock_iheap );

    unsafe_Unsubscribe( id, obj );
  }

  {
    std::tr2::lock_guard<std::tr2::recursive_mutex> lk( obj->_theHistory_lock );
    EventHandler::addr_container_type::iterator r = remove( obj->_ids.begin(), obj->_ids.end(), id );
    obj->_ids.erase( r, obj->_ids.end() );
  }

  cache_clear( obj );
}

void EvManager::cache_clear( EventHandler* obj )
{

  {
    std::tr2::lock_guard<std::tr2::recursive_mutex> lk( obj->_theHistory_lock );
    if ( !obj->_ids.empty() ) {
      return;
    }
  }

  for ( ; ; ) {
    lock_guard<mutex> lk(pheap_lock);

    p_heap_type::iterator i = pheap.find( obj );
    if ( i == pheap.end() ) {
      break;
    }
    
    if ( !i->second.lock->try_lock() ) {
      continue;
    }
      
    std::tr2::mutex* m = 0;
    std::swap( m, i->second.lock );
    pheap.erase( i );

    m->unlock();
    // _cnd_retry.notify_one();
    delete m;
    break;
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

__FIT_DECLSPEC void EvManager::push( const Event& e )
{
  try {
    EventHandler* object = 0;
    bool obj_locked = false;
    mutex* mlk = 0;

    basic_read_lock<rw_mutex> lk(_lock_heap);
    // any object can't be removed from heap within lk scope
    local_heap_type::iterator i = heap.find( e.dest() );
    if ( i == heap.end() ) {
      throw invalid_argument( addr_unknown );
    }
    if ( i->second.empty() ) { // object hasn't StEM address 
      return; // Unsubscribe in progress?
    }

    object = i->second.top().second.second; // target object

    unique_lock<mutex> plk(pheap_lock);

    ev_queue& q = pheap[object];
    if ( q.lock == 0 ) {
      q.lock = new mutex;
    }
    
    q.evs.push( e );

    _cnd_queue.notify_one(); // condition ?!

    // (i->second.top().second.first & (EvManager::remote | EvManager::nosend)) != 0
      
#ifdef __FIT_STEM_TRACE
    try {
      lock_guard<mutex> lk(_lock_tr);
      if ( _trs != 0 && _trs->good() && (_trflags & tracesend) ) {
        // it's target, not source
        // object-> commented, because usage of object's methods
        // unsafe here
        *_trs /* << xmt::demangle( object->classtype().name() ) */
          << " (" << object << ") [target]\n";
        if ( (_trflags & tracetime) ) {
          *_trs << std::tr2::get_system_time().nanoseconds_since_epoch().count();
        }
        int f = _trs->flags();
        *_trs << "\tSend " << std::hex << std::showbase << e.code() << std::dec << endl;
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

#if 0
    // process events to 'remotes' here (on stack)
    // may lead to stalling, if send delay (can't deliver
    // packet immediately)
#endif // 0
  }
  catch ( std::logic_error& err ) {
    try {
      lock_guard<mutex> lk(_lock_tr);
      if ( _trs != 0 && _trs->good() && ((_trflags & (tracefault | tracesend)) == (tracefault | tracesend)) ) {
        if ( (_trflags & tracetime) ) {
          *_trs << std::tr2::get_system_time().nanoseconds_since_epoch().count();
        }
        int f = _trs->flags();
        *_trs << "\tSend fail " << std::hex << std::showbase << e.code() << std::dec << endl;
#ifdef STLPORT
        _trs->flags( f );
#else
        _trs->flags( static_cast<std::_Ios_Fmtflags>(f) );
#endif
      }
    }
    catch ( ... ) {
    }
  }
  catch ( std::runtime_error& err ) {
    try {
      lock_guard<mutex> lk(_lock_tr);
      if ( _trs != 0 && _trs->good() && (_trflags & tracefault) ) {
        *_trs << HERE << ' ' << err.what() << endl;
      }
    }
    catch ( ... ) {
    }
  }
  catch ( ... ) {
    try {
      lock_guard<mutex> lk(_lock_tr);
      if ( _trs != 0 && _trs->good() && (_trflags & tracefault) ) {
        *_trs << HERE << " unknown, uncatched exception" << endl;
      }
    }
    catch ( ... ) {
    }
  }
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

/*
 * for all objects with flag 'remote', try to annotate l
 * on remote side and ask to annotate r on local side
 * (if r is available on remote, annotate it on local side)
 */
void EvManager::annotate_remotes( const addr_type& l, const addr_type& r )
{
  Event ev( EV_STEM_ANNOTATION );

  ev.dest( badaddr ); // special: no destination
  ev.src( l );        // will be annotated in NetTransport::connect
                      // or NetTransportMgr::_loop

  // NetTransport_base::Dispatch( ev );

  basic_read_lock<rw_mutex> lk(_lock_heap);
  for ( local_heap_type::const_iterator i = heap.begin(); i != heap.end(); ++i ) {
    if ( !i->second.empty() ) {
      if ( (i->second.top().second.first & EvManager::remote) != 0 ) {
        i->second.top().second.second->Dispatch( ev );
      }
    }
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

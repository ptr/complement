// -*- C++ -*- Time-stamp: <10/07/12 09:11:36 ptr>

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

  for ( ; ; ) {
    unique_lock<mutex> plk(me.pheap_lock);
    me._cnd_queue.wait( plk, me.not_empty );

    if ( me._dispatch_stop ) {
      return;
    }

    obj = me.plist.front().first;
    n = me.plist.front().second;

    p_heap_type::iterator i = me.pheap.find( obj );
    if ( i != me.pheap.end() ) {
      if ( i->second.lock->try_lock() ) {
        me.plist.pop_front();
      } else {
        if ( me.plist.size() > 1 ) {
          std::list<std::pair<EventHandler*,int> >::iterator j = me.plist.begin();
          ++j;
          obj = j->first;
          n = j->second;
          i = me.pheap.find( obj );
          if ( i != me.pheap.end() ) {
            if ( i->second.lock->try_lock() ) {
              me.plist.erase( j ); // ok, it ready
            } else {
              // i->second.lock->lock(); // just wait
              // i->second.lock->unlock();
              // may be check next in plist?
              cerr << HERE << endl;
              continue; // locked too, unlock pheap and try again
            }
          } else { // no object
            me.plist.erase( j );
            continue;
          }
        } else {
          // plk.unlock();
          // i->second.lock->lock(); // just wait
          // i->second.lock->unlock();
          // cerr << HERE << endl;
          // me.plist.pop_front();
          continue; // locked, unlock pheap and try again
        }
      }

      list<Event> ev;
      list<Event>::iterator r = /* i->second.evs.end(); */ i->second.evs.begin();
      while ( n-- > 0 && r != i->second.evs.end() ) {
        ++r;
      }
      ev.splice( ev.begin(), i->second.evs, i->second.evs.begin(), r );

      mutex* mlk = i->second.lock; // it locked here

      plk.unlock();
      while ( !ev.empty() ) {
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
              obj->DispatchTrace( ev.front(), *me._trs );
              *me._trs << endl;
            }
          }
          catch ( ... ) {
          }
#endif // __FIT_STEM_TRACE
          if ( !obj->Dispatch( ev.front() ) ) {
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
              obj->DispatchTrace( ev.front(), *me._trs );
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
              obj->DispatchTrace( ev.front(), *me._trs );
              *me._trs << endl;
            }
          }
          catch ( ... ) {
          }
        }
        ev.pop_front();
      }

      plk.lock();
      i = me.pheap.find( obj );
      if ( i != me.pheap.end() ) {
        if ( i->second.evs.empty() ) {
          mutex* m = 0;

          swap( m, i->second.lock );
          me.pheap.erase( i );
          m->unlock();

          plk.unlock();
          delete m;
        } else {
          i->second.lock->unlock();
        }
      } else {
        mlk->unlock(); // see cache_clear() for dtor
      }

      // obj->_theHistory_lock.unlock();
    } else {
      me.plist.pop_front();
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
  cache_clear( obj );
}

void EvManager::cache_clear( EventHandler* obj )
{
  // std::tr2::lock_guard<std::tr2::recursive_mutex> hlk( obj->_theHistory_lock );
  if ( !obj->_ids.empty() ) {
    return;
  }

  pheap_lock.lock();

  p_heap_type::iterator i = pheap.find( obj );
  if ( i != pheap.end() ) {
    std::tr2::mutex* m = 0;

    std::swap( m, i->second.lock );
    pheap.erase( i );
    if ( !plist.empty() && (plist.front().first == obj) ) {
      plist.pop_front();
    }

    pheap_lock.unlock(); // before m->lock();

    m->lock();
    // This lock to be sure that only one owner remains: here
    m->unlock();
    
    delete m;
  } else {
    pheap_lock.unlock();
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

  for ( EventHandler::addr_container_type::iterator i = obj->_ids.begin(); i != obj->_ids.end(); ) {
    if ( *i == id ) {
      obj->_ids.erase( i++ );
    } else {
      ++i;
    }
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

    {
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

      if ( (i->second.top().second.first & (EvManager::remote | EvManager::nosend)) != 0 ) {
        unique_lock<mutex> plk(pheap_lock);
        if ( pheap[object].lock == 0 ) {
          pheap[object].lock = new mutex();
        }
        mlk = pheap[object].lock;
        obj_locked = !mlk->try_lock();
        // obj_locked = !object->_theHistory_lock.try_lock();
        // obj_locked = false;
      } else {
        obj_locked = true;
      }

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

      if ( obj_locked ) {
        lock_guard<mutex> plk(pheap_lock);
        if ( plist.empty() || (plist.back().first != object) ) {
          plist.push_back(make_pair(object,1));
        } else {
          ++plist.back().second;
        }
        pheap[object].evs.push_back( e );
        if ( pheap[object].lock == 0 ) {
          pheap[object].lock = new mutex();
        }
        _cnd_queue.notify_all();
        return;
      }
    }

    // process events to 'remotes' here (on stack)
    // may lead to stalling, if send delay (can't deliver
    // packet immediately)

    // object already locked here, see loop above:
    // std::tr2::lock_guard<std::tr2::recursive_mutex> lkh( object->_theHistory_lock, std::tr2::adopt_lock );

    try {
#ifdef __FIT_STEM_TRACE
      try {
        lock_guard<mutex> lk(_lock_tr);
        if ( _trs != 0 && _trs->good() && (_trflags & tracedispatch) ) {
          *_trs << xmt::demangle( object->classtype().name() )
                << " (" << object << ")\n";
          if ( (_trflags & tracetime) ) {
            *_trs << std::tr2::get_system_time().nanoseconds_since_epoch().count();
          }
          object->DispatchTrace( e, *_trs );
          *_trs << endl;
        }
      }
      catch ( ... ) {
      }
#endif // __FIT_STEM_TRACE
      if ( !object->Dispatch( e ) ) { // call dispatcher
        throw std::logic_error( no_catcher );
      }
    }
    catch ( std::logic_error& err ) {
      try {
        lock_guard<mutex> lk(_lock_tr);
        if ( _trs != 0 && _trs->good() && (_trflags & tracefault) ) {
          *_trs << err.what() << "\n"
                << xmt::demangle( object->classtype().name() ) << " (" << object << ")\n";
          if ( (_trflags & tracetime) ) {
            *_trs << std::tr2::get_system_time().nanoseconds_since_epoch().count();
          }
          object->DispatchTrace( e, *_trs );
          *_trs << endl;
        }
      }
      catch ( ... ) {
      }
    }
    catch ( ... ) {
      try {
        lock_guard<mutex> lk(_lock_tr);
        if ( _trs != 0 && _trs->good() && (_trflags & tracefault) ) {
          *_trs << "Unknown, uncatched exception during process:\n"
                << xmt::demangle( object->classtype().name() ) << " (" << object << ")\n";
          if ( (_trflags & tracetime) ) {
            *_trs << std::tr2::get_system_time().nanoseconds_since_epoch().count();
          }
          object->DispatchTrace( e, *_trs );
          *_trs << endl;
        }
      }
      catch ( ... ) {
      }
    }

    lock_guard<mutex> plk(pheap_lock);
    p_heap_type::iterator i = pheap.find( object );
    if ( i != pheap.end() ) {
      if ( i->second.evs.empty() ) {
        mutex* m = 0;

        swap( m, i->second.lock );
        pheap.erase( i );
        m->unlock();
        delete m;
      } else {
        i->second.lock->unlock();
      }
    } else {
      mlk->unlock();
    }
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

void EvManager::sync_call( EventHandler& object, const Event& e )
{
  try {
    std::tr2::lock_guard<std::tr2::recursive_mutex> lk( object._theHistory_lock );

    try {
#ifdef __FIT_STEM_TRACE
      try {
        lock_guard<mutex> lk(_lock_tr);
        if ( _trs != 0 && _trs->good() && (_trflags & tracedispatch) ) {
          *_trs << xmt::demangle( object.classtype().name() )
                << " (" << &object << ")\n";
          if ( (_trflags & tracetime) ) {
            *_trs << std::tr2::get_system_time().nanoseconds_since_epoch().count();
          }
          object.DispatchTrace( e, *_trs );
          *_trs << endl;
        }
      }
      catch ( ... ) {
      }
#endif // __FIT_STEM_TRACE
      if ( !object.Dispatch( e ) ) { // call dispatcher
        throw std::logic_error( no_catcher );
      }
    }
    catch ( std::logic_error& err ) {
      try {
        lock_guard<mutex> lk(_lock_tr);
        if ( _trs != 0 && _trs->good() && (_trflags & tracefault) ) {
          *_trs << err.what() << "\n"
                << xmt::demangle( object.classtype().name() ) << " (" << &object << ")\n";
          if ( (_trflags & tracetime) ) {
            *_trs << std::tr2::get_system_time().nanoseconds_since_epoch().count();
          }
          object.DispatchTrace( e, *_trs );
          *_trs << endl;
        }
      }
      catch ( ... ) {
      }
    }
    catch ( ... ) {
      try {
        lock_guard<mutex> lk(_lock_tr);
        if ( _trs != 0 && _trs->good() && (_trflags & tracefault) ) {
          *_trs << "Unknown, uncatched exception during process:\n"
                << xmt::demangle( object.classtype().name() ) << " (" << &object << ")\n";
          if ( (_trflags & tracetime) ) {
            *_trs << std::tr2::get_system_time().nanoseconds_since_epoch().count();
          }
          object.DispatchTrace( e, *_trs );
          *_trs << endl;
        }
      }
      catch ( ... ) {
      }
    }      
  }
  catch ( std::logic_error& err ) {
    try {
      lock_guard<mutex> lk(_lock_tr);
      if ( _trs != 0 && _trs->good() && (_trflags & tracefault) ) {
        *_trs << HERE << ' ' << err.what() << endl;
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

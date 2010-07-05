// -*- C++ -*- Time-stamp: <10/07/05 13:57:52 ptr>

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
    _trs( 0 )
{
  for ( int k = 0; k < 4; ++k ) {
    nests.push_back( subqueue_container() );
    refs.push_back( nest_ref() );
    refs.back().mgr = this;
    refs.back().q = &nests.back();

    threads.push_back( new thread(_Dispatch_sub, refs.back() ) );
  }
}

__FIT_DECLSPEC EvManager::~EvManager()
{
  stop_queue();
}

bool EvManager::not_finished()
{
  lock_guard<mutex> lk( _lock_queue );
  return !_dispatch_stop;
}

void EvManager::stop_queue()
{
  {
    lock_guard<mutex> lk( _lock_queue );
    _dispatch_stop = true;
    _cnd_queue.notify_all();
  }

  for ( nests_type::iterator i = nests.begin(); i != nests.end(); ++i ) {
    lock_guard<mutex> lk(i->lock);
    i->stop = true;
    i->q.clear(); // erase all unprocessed
    i->cnd.notify_one();
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

void EvManager::_Dispatch_sub( nest_ref p )
{
  EvManager& me = *p.mgr;
  subqueue_container& sq = *p.q;
  subqueue_type& q = sq.q;
  mutex& lock = sq.lock;
  condition_variable& cnd = sq.cnd;

  subqueue_condition qcnd( sq );

  Event ev;

  do {
    unique_lock<mutex> lk( lock );
    while ( !q.empty() ) {
      swap( ev, q.front() );
      q.pop_front();
      lk.unlock();
      me.Send( ev );
      lk.lock();
    }
    cnd.wait( lk, qcnd );
  } while ( me.not_finished() );
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
  // {
  //   lock_guard<rw_mutex> _x1( _lock_heap );

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
  // }

  // {
  //   std::tr2::lock_guard<std::tr2::mutex> lk( _lock_iheap );

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
  // }
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
  // process events to 'remotes' here (on stack)
  // may lead t stalling, if send delay (can't deliver
  // packet immediately)
  try {
    EventHandler* object = 0;
    bool obj_locked = false;
    for ( ; ; ) {
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

        if ( (i->second.top().second.first & (EvManager::remote | EvManager::nosend) ) == 0 ) {
          vector<int> nest_sz( refs.size() );
          vector<int>::iterator m, k;

          fill( nest_sz.begin(), nest_sz.end(), 0 );
          m = nest_sz.begin();
          for ( nests_type::iterator i = nests.begin(); i != nests.end(); ++i, ++m ) {
            lock_guard<mutex> lk( i->lock );
            for ( subqueue_type::const_iterator j = i->q.begin(); j != i->q.end(); ++j ) {
              if ( j->dest() == e.dest() ) {
                i->q.push_back( e );
                i->cnd.notify_one();
                return;
              }
            }
            *m = i->q.size();
          }

          m = min_element( nest_sz.begin(), nest_sz.end() );

          k = nest_sz.begin();

          for ( nests_type::iterator i = nests.begin(); i != nests.end(); ++i, ++k ) {
            if ( m == k ) {
              lock_guard<mutex> lk( i->lock );
              i->q.push_back( e );
              i->cnd.notify_one();        
              break;
            }
          }

          return;
        }

        // process event here

        object = i->second.top().second.second; // target object

        obj_locked = !object->_theHistory_lock.try_lock();
        // if lock object here failed, then something already lock it,
        // possible during attempt to unsubscribe and destroy;
        // don't lock it, unlock heap and repeate search object:
        // may be it already removed
      }
      if ( !obj_locked ) {
        break;
      }
      std::tr2::this_thread::yield();
    }

    // object already locked here, see loop above:
    std::tr2::lock_guard<std::tr2::recursive_mutex> lk( object->_theHistory_lock, std::tr2::adopt_lock );

    try {
#ifdef __FIT_STEM_TRACE
      try {
        lock_guard<mutex> lk(_lock_tr);
        if ( _trs != 0 && _trs->good() && (_trflags & tracedispatch) ) {
          *_trs << xmt::demangle( object->classtype().name() )
                << " (" << object << ")\n";
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
          object->DispatchTrace( e, *_trs );
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

void EvManager::Send( const Event& e )
{
  try {
    EventHandler* object = 0;
    bool obj_locked = false;
    for ( ; ; ) {
      {
        basic_read_lock<rw_mutex> lk(_lock_heap);
        // any object can't be removed from heap within lk scope
        local_heap_type::iterator i = heap.find( e.dest() );
        if ( i == heap.end() ) { // destination not found
          throw invalid_argument( addr_unknown );
        }
        if ( i->second.empty() ) { // object hasn't StEM address 
          return; // Unsubscribe in progress?
        }
        object = i->second.top().second.second; // target object
        obj_locked = !object->_theHistory_lock.try_lock();
        // if lock object here failed, then something already lock it,
        // possible during attempt to unsubscribe and destroy;
        // don't lock it, unlock heap and repeate search object:
        // may be it already removed
      }
      if ( !obj_locked ) {
        break;
      }
      std::tr2::this_thread::yield();
    }

    // object already locked here, see loop above:
    std::tr2::lock_guard<std::tr2::recursive_mutex> lk( object->_theHistory_lock, std::tr2::adopt_lock );

    try {
#ifdef __FIT_STEM_TRACE
      try {
        lock_guard<mutex> lk(_lock_tr);
        if ( _trs != 0 && _trs->good() && (_trflags & tracedispatch) ) {
          *_trs << xmt::demangle( object->classtype().name() )
                << " (" << object << ")\n";
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
          object->DispatchTrace( e, *_trs );
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

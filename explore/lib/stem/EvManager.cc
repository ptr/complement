// -*- C++ -*- Time-stamp: <09/05/05 11:06:49 ptr>

/*
 *
 * Copyright (c) 1995-1999, 2002, 2003, 2005-2009
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

namespace stem {

using namespace std;
using namespace std::tr2;

const addr_type& badaddr = xmt::nil_uuid;
const code_type badcode  = 0xffffffff;

std::string EvManager::inv_key_str( "invalid key" );

__FIT_DECLSPEC EvManager::EvManager() :
    not_empty( *this ),
    _id( xmt::uid() ),
    _dispatch_stop( false ),
    _trflags( 0 ),
    _trs( 0 ),
    _ev_queue_thr( _Dispatch, this )
{
  // _cnd_queue.set( false );
  // _ev_queue_thr.launch( _Dispatch, this );
}

__FIT_DECLSPEC EvManager::~EvManager()
{
  {
    lock_guard<mutex> lk( _lock_queue );
    _dispatch_stop = true;
    _cnd_queue.notify_one();
  }

  _ev_queue_thr.join();
}

bool EvManager::not_finished()
{
  lock_guard<mutex> lk( _lock_queue );
  return !_dispatch_stop;
}

void EvManager::_Dispatch( EvManager* p )
{
  EvManager& me = *p;
  mutex& lq = me._lock_queue;
  queue_type& in_ev_queue = me.in_ev_queue;
  queue_type& out_ev_queue = me.out_ev_queue;

  while ( me.not_finished() ) {
    {
      unique_lock<mutex> lk( lq );
      in_ev_queue.swap( out_ev_queue );
#ifdef __FIT_STEM_TRACE
      try {
        lock_guard<mutex> lk(me._lock_tr);
        if ( me._trs != 0 && me._trs->good() && (me._trflags & tracedispatch) ) {
          ios_base::fmtflags f = me._trs->flags( ios_base::hex | ios_base::showbase );
          *me._trs << "EvManager queues swapped" << endl;
          me._trs->flags( f );
        }
      }
      catch ( ... ) {
      }
#endif // __FIT_STEM_TRACE
    }
    
    while ( !out_ev_queue.empty() ) {
      me.Send( out_ev_queue.front() );
      out_ev_queue.pop_front();
    }
    {
      unique_lock<mutex> lk( lq );
      me._cnd_queue.wait( lk, me.not_empty );
    }
  }

#ifdef __FIT_STEM_TRACE
  try {
    lock_guard<mutex> lk(me._lock_tr);
    if ( me._trs != 0 && me._trs->good() && (me._trflags & tracedispatch) ) {
      ios_base::fmtflags f = me._trs->flags( ios_base::hex | ios_base::showbase );
      *me._trs << "EvManager Dispatch loop finished" << endl;
      me._trs->flags( f );
    }
  }
  catch ( ... ) {
  }
#endif // __FIT_STEM_TRACE

  // try process the rest of queue, if not empty
  {
    unique_lock<mutex> lk( lq );
    in_ev_queue.swap( out_ev_queue );
  }
  while ( !out_ev_queue.empty() ) {
    me.Send( out_ev_queue.front() );
    out_ev_queue.pop_front();
  }

#ifdef __FIT_STEM_TRACE
  try {
    lock_guard<mutex> lk(me._lock_tr);
    if ( me._trs != 0 && me._trs->good() && (me._trflags & tracedispatch) ) {
      ios_base::fmtflags f = me._trs->flags( ios_base::hex | ios_base::showbase );
      *me._trs << "EvManager stop Dispatch" << endl;
      me._trs->flags( f );
    }
  }
  catch ( ... ) {
  }
#endif // __FIT_STEM_TRACE
}

__FIT_DECLSPEC
void EvManager::Subscribe( const addr_type& id, EventHandler* object, int nice )
{
  lock_guard<mutex> lk( _lock_heap );
  heap[id].push( make_pair( nice, object) );
}

__FIT_DECLSPEC
void EvManager::Subscribe( const addr_type& id, EventHandler* object, const std::string& info, int nice )
{
  {
    lock_guard<mutex> lk( _lock_heap );
    heap[id].push( make_pair( nice, object) );
  }

  annotate( id, info );
}

__FIT_DECLSPEC
void EvManager::Subscribe( const addr_type& id, EventHandler* object, const char* info, int nice )
{
  {
    lock_guard<mutex> lk( _lock_heap );
    heap[id].push( make_pair( nice, object) );
  }

  annotate( id, info );
}

__FIT_DECLSPEC
void EvManager::Unsubscribe( const addr_type& id, EventHandler* obj )
{
  {
    lock_guard<mutex> _x1( _lock_heap );

    local_heap_type::iterator i = heap.find( id );
    if ( i != heap.end() ) {
      if ( i->second.size() == 1 ) {
        heap.erase( i );
      } else {
        handlers_type tmp;
        swap( i->second, tmp );
        while ( !tmp.empty() ) {
          weighted_handler_type handler = tmp.top();
          if ( handler.second != obj ) {
            i->second.push( handler );
          }
          tmp.pop();
        }
      }
    }
  }
  {
    std::tr2::lock_guard<std::tr2::mutex> lk( _lock_iheap );

    for ( info_heap_type::iterator i = iheap.begin(); i != iheap.end(); ) {
      for ( addr_collection_type::iterator j = i->second.begin(); j != i->second.end(); ++j ) {
        if ( *j == id ) {
          lock_guard<mutex> _x1( _lock_heap );

          local_heap_type::iterator k = heap.find( id );
          if ( k == heap.end() ) { // all objects with this addr removed
            i->second.erase( j );
          }
          break;
        }
      }

      if ( i->second.empty() ) {
        iheap.erase( i++ );
      } else {
        ++i;
      }
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

void EvManager::settrs( std::ostream *s )
{
  lock_guard<mutex> _x1( _lock_tr );
  _trs = s;
}

__FIT_DECLSPEC void EvManager::push( const Event& e )
{
  std::tr2::lock_guard<std::tr2::mutex> lk( _lock_queue );
  in_ev_queue.push_back( e );
  _cnd_queue.notify_one();
#ifdef __FIT_STEM_TRACE
  try {
    lock_guard<mutex> lk(_lock_tr);
    if ( _trs != 0 && _trs->good() && (_trflags & tracedispatch) ) {
      ios_base::fmtflags f = _trs->flags( ios_base::hex | ios_base::showbase );
      *_trs << "EvManager push event " << e.code() << " to queue" << endl;
      _trs->flags( f );
    }
  }
  catch ( ... ) {
  }
#endif // __FIT_STEM_TRACE
}

void EvManager::Send( const Event& e )
{
  try {
    _lock_heap.lock();
    local_heap_type::iterator i = heap.find( e.dest() );
    if ( i == heap.end() ) { // destination not found
      throw invalid_argument( string("address unknown") );
    }
    EventHandler* object = i->second.top().second; // target object
    _lock_heap.unlock();

    try {
#ifdef __FIT_STEM_TRACE
      try {
        lock_guard<mutex> lk(_lock_tr);
        if ( _trs != 0 && _trs->good() && (_trflags & tracedispatch) ) {
          *_trs << object->classtype().name()
                << " (" << object << ")\n";
          object->DispatchTrace( e, *_trs );
          *_trs << endl;
        }
      }
      catch ( ... ) {
      }
#endif // __FIT_STEM_TRACE
      if ( !object->Dispatch( e ) ) { // call dispatcher
        throw std::logic_error( "no catcher for event" );
      }
    }
    catch ( std::logic_error& err ) {
      try {
        lock_guard<mutex> lk(_lock_tr);
        if ( _trs != 0 && _trs->good() && (_trflags & tracefault) ) {
          *_trs << err.what() << "\n"
                << object->classtype().name() << " (" << object << ")\n";
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
                << object->classtype().name() << " (" << object << ")\n";
          object->DispatchTrace( e, *_trs );
          *_trs << endl;
        }
      }
      catch ( ... ) {
      }
    }      
  }
  catch ( std::logic_error& err ) {
// #ifdef __FIT_STEM_TRACE
    try {
      lock_guard<mutex> lk(_lock_tr);
      if ( _trs != 0 && _trs->good() && (_trflags & tracefault) ) {
        *_trs << err.what() << "\n"
              << __FILE__ << ":" << __LINE__ << endl;
      }
    }
    catch ( ... ) {
    }
// #endif // __FIT_STEM_TRACE
    _lock_heap.unlock();
  }
  catch ( std::runtime_error& err ) {
// #ifdef __FIT_STEM_TRACE
    try {
      lock_guard<mutex> lk(_lock_tr);
      if ( _trs != 0 && _trs->good() && (_trflags & tracefault) ) {
        *_trs << err.what() << " "
              << __FILE__ << ":" << __LINE__ << endl;
      }
    }
    catch ( ... ) {
    }
// #endif // __FIT_STEM_TRACE
    _lock_heap.unlock();
  }
  catch ( ... ) {
// #ifdef __FIT_STEM_TRACE
    try {
      scoped_lock lk(_lock_tr);
      if ( _trs != 0 && _trs->good() && (_trflags & tracefault) ) {
        *_trs << "Unknown, uncatched exception: "
              << __FILE__ << ":" << __LINE__ << endl;
      }
    }
    catch ( ... ) {
    }
// #endif // __FIT_STEM_TRACE
    _lock_heap.unlock();
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
  ios_base::fmtflags f = s.flags( ios_base::hex | ios_base::showbase );
  s << "Local map:\n";

  // s << hex << showbase;
  {
    lock_guard<mutex> lk( _lock_heap );

    for ( local_heap_type::const_iterator i = heap.begin(); i != heap.end(); ++i ) {
      s << i->first << "\t=> " << i->second.top().second << "\n";
    }
  }

  s << "\nInfo map:\n";
  {
    lock_guard<mutex> lk( _lock_iheap );

    for ( info_heap_type::const_iterator i = iheap.begin(); i != iheap.end(); ++i ) {
      s << i->first << "\t=> '";
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
  s.flags( f );

  return s;
}

} // namespace stem

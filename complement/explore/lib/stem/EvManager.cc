// -*- C++ -*- Time-stamp: <06/11/24 11:59:21 ptr>

/*
 *
 * Copyright (c) 1995-1999, 2002, 2003, 2005, 2006
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

namespace stem {

#ifndef WIN32
const addr_type badaddr    = 0xffffffff;
const key_type  badkey     = 0xffffffff;
const code_type badcode    = static_cast<code_type>(-1);
const addr_type extbit     = 0x80000000;
const addr_type ns_addr    = 0x00000001;
#endif

#ifdef WIN32
__PG_DECLSPEC addr_type badaddr    = 0xffffffff;
__PG_DECLSPEC key_type  badkey     = 0xffffffff;
__PG_DECLSPEC code_type badcode    = static_cast<code_type>(-1);
__PG_DECLSPEC addr_type extbit     = 0x80000000;
__PG_DECLSPEC addr_type ns_addr    = 0x00000001;
#endif

const           addr_type beglocaddr = 0x00000100;
const           addr_type endlocaddr = 0x3fffffff;
const           addr_type begextaddr = extbit;
const           addr_type endextaddr = 0xbfffffff;

std::string EvManager::inv_key_str( "invalid key" );

__FIT_DECLSPEC EvManager::EvManager() :
    _low( beglocaddr ),
    _high( endlocaddr ),
    _id( _low ),
    _x_low( begextaddr ),
    _x_high( endextaddr ),
    _x_id( _x_low ),
    _dispatch_stop( false )
{
// #ifndef __hpux
  _cnd_queue.set( false );
  _ev_queue_thr.launch( _Dispatch, this );
// #endif
}

__FIT_DECLSPEC EvManager::~EvManager()
{
  _ev_queue_dispatch_guard.lock();
  _dispatch_stop = true;
  _cnd_queue.set( true );
  _ev_queue_dispatch_guard.unlock();
  _ev_queue_thr.join();
}

bool EvManager::not_finished()
{
  xmt::LockerSpin _lk( _ev_queue_dispatch_guard );
  return !_dispatch_stop;
}

xmt::Thread::ret_code EvManager::_Dispatch( void *p )
{
  EvManager& me = *reinterpret_cast<EvManager *>(p);
  xmt::Thread::ret_code rt;
  rt.iword = 0;

  while ( me.not_finished() ) {
    MT_LOCK( me._lock_queue );
    swap( me.in_ev_queue, me.out_ev_queue );
    MT_UNLOCK( me._lock_queue );
    while ( !me.out_ev_queue.empty() ) {
      me.Send( me.out_ev_queue.front() );
      me.out_ev_queue.pop();
    }
    MT_LOCK( me._lock_queue );
    if ( me.in_ev_queue.empty() && me.not_finished() ) {
      me._cnd_queue.set( false );
      MT_UNLOCK( me._lock_queue );
      me._cnd_queue.try_wait();
    } else {
      MT_UNLOCK( me._lock_queue );
    }
  }

  return rt;
}

__FIT_DECLSPEC
addr_type EvManager::Subscribe( EventHandler *object, const std::string& info )
{
  addr_type id;
  {
    Locker _x1( _lock_heap );
    id = create_unique();
    heap[id] = object;
  }
  {
    Locker _x1( _lock_iheap );
    iheap[id] = info;
  }
  
  return id;
}

__FIT_DECLSPEC
addr_type EvManager::Subscribe( EventHandler *object, const char *info )
{
  addr_type id;
  {
    Locker _x1( _lock_heap );
    id = create_unique();
    heap[id] = object;
  }
  if ( info ) {
    Locker _x1( _lock_iheap );
    iheap[id] = info;
  }
  
  return id;
}

__FIT_DECLSPEC
addr_type EvManager::SubscribeID( addr_type id, EventHandler *object,
                                  const std::string& info )
{
  if ( (id & extbit) ) {
    return badaddr;
  } else {
    Locker _x1( _lock_heap );
    if ( unsafe_is_avail( id ) ) {
      return badaddr;
    }
    heap[id] = object;
  }

  Locker _x1( _lock_iheap );
  iheap[id] = info;

  return id;
}

__FIT_DECLSPEC
addr_type EvManager::SubscribeID( addr_type id, EventHandler *object,
                                  const char *info )
{
  if ( (id & extbit) ) {
    return badaddr;
  } else {
    Locker _x1( _lock_heap );
    if ( unsafe_is_avail( id ) ) {
      return badaddr;
    }
    heap[id] = object;
  }
  if ( info ) {
    Locker _x1( _lock_iheap );
    iheap[id] = info;
  }

  return id;
}

__FIT_DECLSPEC
addr_type EvManager::SubscribeRemote( const detail::transport& tr,
                                      const gaddr_type& addr,
                                      const std::string& info )
{
  addr_type id;
  {
    Locker _x1( _lock_xheap );
    id = create_unique_x();
    _ex_heap[id] = addr;
    _ui_heap[addr] = id;
    _tr_heap.insert( make_pair( addr, tr ) );
    _ch_heap.insert( make_pair( tr.link, addr ) );
  }
  {
    Locker _x1( _lock_iheap );
    iheap[id] = info;
  }

  return id;
}

__FIT_DECLSPEC
addr_type EvManager::SubscribeRemote( const detail::transport& tr,
                                      const gaddr_type addr,
                                      const char *info )
{
  addr_type id;
  {
    Locker _x1( _lock_xheap );
    id = create_unique_x();
    _ex_heap[id] = addr;
    _ui_heap[addr] = id;
    _tr_heap.insert( make_pair( addr, tr ) );
    _ch_heap.insert( make_pair( tr.link, addr ) );
  }
  if ( info ) {
    Locker _x1( _lock_iheap );
    iheap[id] = info;
  }

  return id;
}

__FIT_DECLSPEC
bool EvManager::Unsubscribe( addr_type id )
{
  if ( (id & extbit) ) {
    Locker _x1( _lock_xheap );
    gaddr_type& addr = _ex_heap[id];
      
    pair<uuid_tr_heap_type::iterator,uuid_tr_heap_type::iterator> range = _tr_heap.equal_range( addr );
    for ( uuid_tr_heap_type::iterator i = range.first; i != range.second; ++i ) {
      pair<tr_uuid_heap_type::iterator,tr_uuid_heap_type::iterator> ch_range = _ch_heap.equal_range( i->link );
      for ( tr_uuid_heap_type::iterator j = ch_range.first; j != ch_range.second; ) {
        if ( j->second == i->first ) {
          _ch_heap.erase( j++ );
          continue;
        }
        ++j;
      }
    }
    _tr_heap.erase( range.first, range.second );
    _ui_heap.erase( addr );
    _ex_heap.erase( id );
  } else {
    Locker _x1( _lock_heap );
    heap.erase( id );

    // Notify remotes?
  }
  Locker _x1( _lock_iheap );
  iheap.erase( id );

  return true;
}

__FIT_DECLSPEC
void EvManager::Remove( void *channel )
{
  Locker _x1( _lock_xheap );
  Locker _x2( _lock_iheap );
  unsafe_Remove( channel );
}

// Remove references to remote objects, that was announced via 'channel'
// (related, may be, with socket connection)
// from [remote name -> local name] mapping table, and mark related session as
// 'disconnected'.
__FIT_DECLSPEC
void EvManager::unsafe_Remove( void *channel )
{
  pair<tr_uuid_heap_type::iterator,tr_uuid_heap_type::iterator> ch_range = _ch_heap.equal_range( channel );
  for (tr_uuid_heap_type::iterator i = ch_range.first; i != ch_range.second; ++i ) {
    _tr_heap.erase( i->second );
    addr_type address = _ui_heap[i->second];
    _ex_heap.erase( address );
    iheap.erase( address );
    _ui_heap.erase( i->second );
  }
  _ch_heap.erase( ch_range.first, ch_range.second );
}

__FIT_DECLSPEC const detail::transport& EvManager::transport( addr_type id ) const
{
  Locker _x1( _lock_xheap );
  if ( (id & extbit) != 0 ) {
    ext_uuid_heap_type::iterator i = _ex_heap.find( id );
    if ( i == _ex_heap.end() ) {
      throw range_error( string( "no such address" ) );
    }
    pair<uuid_tr_heap_type::iterator,uuid_tr_heap_type::iterator> range = _tr_heap.equal_range( i->second );
    if ( range.first == _tr_heap.end() ) {
      throw range_error( string( "no transport" ) );
    }
    return min_element( range.first, range.second, tr_compare ).second;
  }
  throw range_error( string( "internal address" ) );
}

// Resolve Address -> Object Reference, call Object's dispatcher in case
// of local object, or call appropriate channel delivery function for
// remote object. All outgoing events, and incoming remote events
// (this method allow to forward event from remote object to another remote object,
// i.e. work as 'proxy' with 'transit objects')

void EvManager::Send( const Event& e )
{
  if ( e.dest() & extbit ) { // external object
    try {
      _lock_xheap.lock();
      ext_uuid_heap_type::iterator i = _ex_heap.find( e.dest() );
      if ( i == _ex_heap.end() ) { // destination not found
        throw invalid_argument( string("external address unknown") );
      }

      pair<uuid_tr_heap_type::iterator,uuid_tr_heap_type::iterator> range = _tr_heap.equal_range( i->second );
      if ( range.first == _tr_heap.end() ) {
        throw range_error( string( "no transport" ) );
      }
      detail::transport& tr = min_element( range.first, range.second, tr_compare ).second;
      detail::transport::kind_type k = tr.kind;
      void *link = tr.link;
      uuid_type gaddr_dst( i->second );
      uuid_type gaddr_src;

      ext_uuid_heap_type::iterator j = _ex_heap.find( e.src() );
      if ( j == _ex_heap.end() ) {
        gaddr_type& _gaddr_src = _ex_heap[e.src()];
        _gaddr_src.hid = xmt::hostid();
        _gaddr_src.pid = getpid();
        _gaddr_src.addr = e.src(); // it may be as local as foreign; if e.src()
                                   // is foreign, the object is 'transit object'
        _ui_heap[_gaddr_src] = e.src();
        gaddr_src = _gaddr_src;
      } else {
        gaddr_src = j->second;
      }

      _lock_xheap.unlock();

      switch ( k ) {
        detail::transport::socket_tcp:
          if ( reinterpret_cast<NetTransport_base *>(link)->push( e, gaddr_dst, gaddr_src) ) {
            // if I detect bad connection during writing to net
            // (in the push), I remove this connetion related entries.
            // Required by non-Solaris OS. Unsafe variant allow avoid
            // deadlock here.
            unsafe_Remove( link );
          }
          break;
        detail::transport::unknown:
          break;
        default:
          break;
      }
    }
    catch ( ... ) {
      _lock_xheap.unlock();
    }
  } else { // local object
    try {
      _lock_heap.lock();
      local_heap_type::iterator i = heap.find( e.dest() );
      if ( i == heap.end() ) { // destination not found
        throw invalid_argument( string("address unknown") );
      }
      EventHandler *object = i->second; // target object
      _lock_heap.unlock();

      try {
        object->Dispatch( e ); // call dispatcher
      } 
      catch ( ... ) {
      }      
    }
    catch ( ... ) {
      _lock_heap.unlock();
    }
  }
}

addr_type EvManager::create_unique()
{
  do {
    if ( ++_id > _high ) {
      _id = (_id - _low) % (_high - _low) + _low;
    }
  } while ( heap.find( _id ) != heap.end() );

  return _id;
}

addr_type EvManager::create_unique_x()
{
  do {
    if ( ++_x_id > _x_high ) {
      _x_id = (_x_id - _x_low) % (_x_high - _x_low) + _x_low;
    }
  } while ( heap.find( _x_id ) != heap.end() );

  return _x_id;
}

} // namespace stem

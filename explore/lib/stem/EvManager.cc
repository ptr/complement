// -*- C++ -*- Time-stamp: <03/11/16 22:07:47 ptr>

/*
 *
 * Copyright (c) 1995-1999, 2002, 2003
 * Petr Ovchenkov
 *
 * Copyright (c) 1999-2001
 * ParallelGraphics Ltd.
 *
 * Licensed under the Academic Free License version 2.0
 *
 * This material is provided "as is", with absolutely no warranty expressed
 * or implied. Any use is at your own risk.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.
 */

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "@(#)$Id$"
#  else
#ident "@(#)$Id$"
#  endif
#endif

#ifdef _MSC_VER
#pragma warning( disable : 4804 )
#endif

#include <config/feature.h>
#include "stem/EvManager.h"
#include "stem/NetTransport.h"
#include <iomanip>

namespace EDS {

#ifndef WIN32
const addr_type badaddr    = 0xffffffff;
const key_type  badkey     = 0xffffffff;
const code_type badcode    = static_cast<code_type>(-1);
const addr_type extbit     = 0x80000000;
const addr_type nsaddr     = 0x00000001;
#endif

#ifdef WIN32
__PG_DECLSPEC addr_type badaddr    = 0xffffffff;
__PG_DECLSPEC key_type  badkey     = 0xffffffff;
__PG_DECLSPEC code_type badcode    = static_cast<code_type>(-1);
__PG_DECLSPEC addr_type extbit     = 0x80000000;
__PG_DECLSPEC addr_type nsaddr     = 0x00000001;
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
    _x_id( _x_low )
{
// #ifndef __hpux
  _ev_queue_thr.launch( _Dispatch, this );
// #endif
}

__FIT_DECLSPEC EvManager::~EvManager()
{
  _ev_queue_cond.set( false );
  _ev_queue_thr.join();
}

int EvManager::_Dispatch( void *p )
{
  EvManager& me = *reinterpret_cast<EvManager *>(p);

  while ( me._ev_queue_cond.set() ) {
    MT_LOCK( me._lock_queue );
    // swap( me.in_ev_queue, me.out_ev_queue );
    // _STLP_ASSERT( me.out_ev_queue.empty() );
    // (const_cast<queue_type::container_type&>(me.in_ev_queue._Get_c())).swap( const_cast<queue_type::container_type&>(me.out_ev_queue._Get_c()) );
    swap( me.in_ev_queue, me.out_ev_queue );
    MT_UNLOCK( me._lock_queue );
    while ( !me.out_ev_queue.empty() ) {
      me.Send( me.out_ev_queue.front() );
      me.out_ev_queue.pop();
    }
    MT_LOCK( me._lock_queue );
    if ( me.in_ev_queue.empty() ) {
      MT_UNLOCK( me._lock_queue );
//      timespec t;
//      t.tv_sec = 0;
//      t.tv_nsec = 10000000;
//      __impl::Thread::sleep( &t );
      me._ev_queue_thr.suspend();
    } else {
      MT_UNLOCK( me._lock_queue );
    }
  }

  return 0;
}

__FIT_DECLSPEC
addr_type EvManager::Subscribe( EventHandler *object, const std::string& info )
{
  MT_REENTRANT( _lock_heap, _x1 );
  addr_type id = create_unique();
  __Object_Entry& record = heap[id];
  record.ref = object;
  record.info = info;
  
  return id;
}

__FIT_DECLSPEC
addr_type EvManager::Subscribe( EventHandler *object, const char *info )
{
  MT_REENTRANT( _lock_heap, _x1 );
  addr_type id = create_unique();
  __Object_Entry& record = heap[id];
  record.ref = object;
  if ( info ) {
    record.info = info;
  }
  
  return id;
}

__FIT_DECLSPEC
addr_type EvManager::SubscribeID( addr_type id, EventHandler *object,
                                  const std::string& info )
{
  MT_REENTRANT( _lock_heap, _x1 );
  if ( (id & extbit) || unsafe_is_avail( id ) ) {
    return badaddr;
  }
  __Object_Entry& record = heap[id];
  record.ref = object;
  record.info = info;

  return id;
}

__FIT_DECLSPEC
addr_type EvManager::SubscribeID( addr_type id, EventHandler *object,
                                  const char *info )
{
  MT_REENTRANT( _lock_heap, _x1 );
  if ( (id & extbit) || unsafe_is_avail( id ) ) {
    return badaddr;
  }
  __Object_Entry& record = heap[id];
  record.ref = object;
  if ( info ) {
    record.info = info;
  }

  return id;
}

__FIT_DECLSPEC
addr_type EvManager::SubscribeRemote( NetTransport_base *channel,
                                      addr_type rmkey,
                                      const std::string& info )
{
  MT_REENTRANT( _lock_heap, _x1 );
  addr_type id = create_unique_x();
  __Object_Entry& record = heap[id];
  // record.ref = object;
  record.info = info;
  // _STLP_ASSERT( channel != 0 );
  record.addremote( rmkey, channel );

  return id;
}

__FIT_DECLSPEC
addr_type EvManager::SubscribeRemote( NetTransport_base *channel,
                                      addr_type rmkey,
                                      const char *info )
{
  MT_REENTRANT( _lock_heap, _x1 );
  addr_type id = create_unique_x();
  __Object_Entry& record = heap[id];
  // record.ref = object;
  if ( info ) {
    record.info = info;
  }
  // _STLP_ASSERT( channel != 0 );
  record.addremote( rmkey, channel );

  return id;
}

__FIT_DECLSPEC
bool EvManager::Unsubscribe( addr_type id )
{
  MT_REENTRANT( _lock_heap, _x1 );
  heap.erase( /* (const heap_type::key_type&)*/ id );
  return true; // may be here check object's reference count
}

__FIT_DECLSPEC
void EvManager::Remove( NetTransport_base *channel )
{
  MT_REENTRANT( _lock_heap, _x1 );
  unsafe_Remove( channel );
}

// Remove references to remote objects, that was announced via 'channel'
// (related, may be, with socket connection)
// from [remote name -> local name] mapping table, and mark related session as
// 'disconnected'.
__FIT_DECLSPEC
void EvManager::unsafe_Remove( NetTransport_base *channel )
{
  heap_type::iterator i = heap.begin();

  while ( i != heap.end() ) {
    if ( (*i).second.remote != 0 && (*i).second.remote->channel == channel ) {
      heap.erase( i++ );
    } else {
      ++i;
    }
  }
}

// return session id of object with address 'id' if this is external
// object; otherwise return -1;
__FIT_DECLSPEC key_type EvManager::sid( addr_type id ) const
{
  MT_REENTRANT( _lock_heap, _x1 );
  heap_type::const_iterator i = heap.find( id );
  if ( i == heap.end() || (*i).second.remote == 0 ) {
    return badkey;
  }
  return (*i).second.remote->channel->sid();
}

__FIT_DECLSPEC NetTransport_base *EvManager::transport( addr_type id ) const
{
  MT_REENTRANT( _lock_heap, _x1 );
  heap_type::const_iterator i = heap.find( id );
  if ( i == heap.end() || (*i).second.remote == 0 ) {
    return 0;
  }
  return (*i).second.remote->channel;
}

#if 0
#define _XMB( msg ) \
{ \
  ostringstream ss; \
  ss << msg << "\n" \
     << __FILE__ << ":" << __LINE__ << endl; \
  MessageBox( 0, ss.str().c_str(), "Planet Problem", MB_OK ); \
}

#endif

// Resolve Address -> Object Reference, call Object's dispatcher in case
// of local object, or call appropriate channel delivery function for
// remote object. All outgoing events, and incoming remote events
// (this method allow to forward remote-object-event to another remote-object
void EvManager::Send( const Event& e )
{
  try {
    // Will be useful to block on erase/insert operations...
    MT_LOCK( _lock_heap );
//    _XMB( "MT_LOCK" )
    heap_type::iterator i = heap.find( e.dest() );
    if ( i != heap.end() ) {
      if ( (*i).second.ref != 0 ) { // local delivery
        EventHandler *object = (*i).second.ref;
//       std::cerr << "Local\n";
//        _XMB( "MT_UNLOCK" )
        MT_UNLOCK( _lock_heap );
        try {
          object->Dispatch( e );
        } 
        catch ( ... ) {
        }
      } else { // remote delivery
//       std::cerr << "Remote\n";
        __Remote_Object_Entry *remote = (*i).second.remote;
        // _STLP_ASSERT( remote != 0 );
        addr_type save_dest = e.dest();
        e.dest( remote->key ); // substitute address on remote system
        if ( !remote->channel->push( e ) ) {
          // if I detect bad connection during writing to net
          // (in the push), I remove this connetion related entries.
          // Required by non-Solaris OS. Unsafe variant allow avoid
          // deadlock here.
          unsafe_Remove( remote->channel );
        }
        e.dest( save_dest ); // restore original (may be used more)
//      _XMB( "MT_UNLOCK" )
        MT_UNLOCK( _lock_heap );
      }
    } else {
//      _XMB( "MT_UNLOCK" )
     MT_UNLOCK( _lock_heap );
#if 0
      try {
        std::cerr << "===== EDS: "
                  << std::hex 
                  << std::setiosflags(std::ios_base::showbase)
                  << e.dest()
                  << " not found, source: " << e.src()
                  << ", code " << e.code() << std::dec << endl;
      }
      catch ( ... ) {
      }
#endif
    }
  }
  catch ( ... ) {
//    _XMB( "MT_UNLOCK" )
    MT_UNLOCK( _lock_heap );
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

} // namespace EDS

// -*- C++ -*- Time-stamp: <99/09/14 19:14:16 ptr>
#ident "$SunId$ %Q%"

#ifdef _MSC_VER
#pragma warning( disable : 4804 )
#endif

#ifdef WIN32
#  ifdef _DLL
#    define __EDS_DLL __declspec( dllexport )
#  else
#    define __EDS_DLL
#  endif
#else
#  define __EDS_DLL
#endif

#include <EvManager.h>
#include <NetTransport.h>
#include <iomanip>

namespace EDS {

#ifndef WIN32
const __EDS_DLL addr_type badaddr    = 0xffffffff;
const __EDS_DLL key_type  badkey     = 0xffffffff;
const __EDS_DLL addr_type extbit     = 0x80000000;
const __EDS_DLL addr_type nsaddr     = 0x00000001;
#endif

#ifdef WIN32
__EDS_DLL addr_type badaddr    = 0xffffffff;
__EDS_DLL key_type  badkey     = 0xffffffff;
__EDS_DLL addr_type extbit     = 0x80000000;
__EDS_DLL addr_type nsaddr     = 0x00000001;
#endif

const           addr_type beglocaddr = 0x00000100;
const           addr_type endlocaddr = 0x3fffffff;
const           addr_type begextaddr = extbit;
const           addr_type endextaddr = 0xbfffffff;

std::string EvManager::inv_key_str( "invalid key" );

__EDS_DLL EvManager::EvManager() :
    _low( beglocaddr ),
    _high( endlocaddr ),
    _id( _low ),
    _x_low( begextaddr ),
    _x_high( endextaddr ),
    _x_id( _x_low )
{ }

__EDS_DLL
addr_type EvManager::Subscribe( EventHandler *object, const std::string& info )
{
  MT_REENTRANT( _lock_heap, _1 );
  addr_type id = create_unique();
  __Object_Entry& record = heap[id];
  record.ref = object;
  record.info = info;
  
  return id;
}

__EDS_DLL
addr_type EvManager::Subscribe( EventHandler *object, const char *info )
{
  MT_REENTRANT( _lock_heap, _1 );
  addr_type id = create_unique();
  __Object_Entry& record = heap[id];
  record.ref = object;
  if ( info ) {
    record.info = info;
  }
  
  return id;
}

__EDS_DLL
addr_type EvManager::SubscribeID( addr_type id, EventHandler *object,
                                  const std::string& info )
{
  MT_REENTRANT( _lock_heap, _1 );
  if ( (id & extbit) || is_avail( id ) ) {
    return badaddr;
  }
  __Object_Entry& record = heap[id];
  record.ref = object;
  record.info = info;

  return id;
}

__EDS_DLL
addr_type EvManager::SubscribeID( addr_type id, EventHandler *object,
                                  const char *info )
{
  MT_REENTRANT( _lock_heap, _1 );
  if ( (id & extbit) || is_avail( id ) ) {
    return badaddr;
  }
  __Object_Entry& record = heap[id];
  record.ref = object;
  if ( info ) {
    record.info = info;
  }

  return id;
}

__EDS_DLL
addr_type EvManager::SubscribeRemote( NetTransport_base *channel,
                                      addr_type rmkey,
                                      const std::string& info )
{
  MT_REENTRANT( _lock_heap, _1 );
  addr_type id = create_unique_x();
  __Object_Entry& record = heap[id];
  // record.ref = object;
  record.info = info;
  __stl_assert( channel != 0 );
  record.addremote( rmkey, channel );

  return id;
}

__EDS_DLL
addr_type EvManager::SubscribeRemote( NetTransport_base *channel,
                                      addr_type rmkey,
                                      const char *info )
{
  MT_REENTRANT( _lock_heap, _1 );
  addr_type id = create_unique_x();
  __Object_Entry& record = heap[id];
  // record.ref = object;
  if ( info ) {
    record.info = info;
  }
  __stl_assert( channel != 0 );
  record.addremote( rmkey, channel );

  return id;
}

__EDS_DLL
bool EvManager::Unsubscribe( addr_type id )
{
  // MT_REENTRANT( _lock_heap, _1 );
  heap.erase( /* (const heap_type::key_type&)*/ id );
  return true; // may be here check object's reference count
}

// Remove references to remote objects, that was announced via 'channel'
// (related, may be, with socket connection)
// from [remote name -> local name] mapping table, and mark related session as
// 'disconnected'.
void EvManager::Remove( NetTransport_base *channel )
{
  MT_LOCK( _lock_heap );
  heap_type::iterator i = heap.begin();

  while ( i != heap.end() ) {
    if ( (*i).second.remote != 0 && (*i).second.remote->channel == channel ) {
      heap.erase( i++ );
    } else {
      ++i;
    }
  }
  MT_UNLOCK( _lock_heap );
}

// return session id of object with address 'id' if this is external
// object; otherwise return -1;
key_type EvManager::sid( addr_type id ) const
{
  MT_REENTRANT( _lock_heap, _1 );
  heap_type::const_iterator i = heap.find( id );
  if ( i == heap.end() || (*i).second.remote == 0 ) {
    return badkey;
  }
  return (*i).second.remote->channel->sid();
}

// Resolve Address -> Object Reference, call Object's dispatcher in case
// of local object, or call appropriate channel delivery function for
// remote object. All outgoing events, and incoming remote events
// (this method allow to forward remote-object-event to another remote-object
void EvManager::Send( const Event& e )
{
  // Will be useful to block on erase/insert operations...
  // MT_REENTRANT( _lock_heap, _1 );
  heap_type::iterator i = heap.find( e.dest() );
  if ( i != heap.end() ) {
    if ( (*i).second.ref != 0 ) { // local delivery
//       std::cerr << "Local\n";
      (*i).second.ref->Dispatch( e );
    } else { // remote delivery
//       std::cerr << "Remote\n";
      __stl_assert( (*i).second.remote != 0 );
      addr_type save_dest = e.dest();
      e.dest( (*i).second.remote->key ); // substitute address on remote system
      (*i).second.remote->channel->push( e );
      e.dest( save_dest ); // restore original (may be used more)
    }
  } else {
    std::cerr << "===== EDS: "
              << std::hex << std::setiosflags(std::ios_base::showbase)
              << e.dest()
              << " not found\n" << std::dec;
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

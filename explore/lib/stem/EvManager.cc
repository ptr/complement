// -*- C++ -*- Time-stamp: <99/08/18 22:56:55 ptr>
#ident "%Z% $Date$ $Revision$ $RCSfile$ %Q%"

#include <EvManager.h>
#include <NetTransport.h>

namespace EDS {

std::string EvManager::inv_key_str( "invalid key" );

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
EvSessionManager::key_type EvManager::sid( const key_type& id ) const
{
  MT_REENTRANT( _lock_heap, _1 );
  heap_type::const_iterator i = heap.find( id );
  if ( i == heap.end() || (*i).second.remote == 0 ) {
    return static_cast<EvSessionManager::key_type>(-1);
  }
  return (*i).second.remote->channel->sid();
}

// Local access to object from remote call: only local object may be here;
// This function used only from NetTransport after mapping remote object
// address to its local address. (Incoming remote events only).
void EvManager::Dispatch( const Event& e )
{
  heap_type::iterator i = heap.find( e.dest() );
  if ( i != heap.end() && (*i).second.ref != 0 ) {
    (*i).second.ref->Dispatch( e );
  }
}

// Resolve Address -> Object Reference, call Object's dispatcher in case
// of local object, or call appropriate channel delivery function for
// remote object. (Outcoming local and remote events).
void EvManager::Send( const Event& e, const EvManager::key_type& src_id )
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
      (*i).second.remote->channel->push( e, (*i).second.remote->key, src_id );
    }
  } else {
    std::cerr << "===================== Not found\n";
  }
}

EvManager::key_type EvManager::create_unique()
{
  do {
    if ( ++_id > _high ) {
      _id = (_id - _low) % (_high - _low) + _low;
    }
  } while ( heap.find( _id ) != heap.end() );

  return _id;
}

} // namespace EDS

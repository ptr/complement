// -*- C++ -*- Time-stamp: <99/04/06 19:15:34 ptr>
#ident "%Z% $Date$ $Revision$ $RCSfile$ %Q%"

#include <EvManager.h>
#include <NetTransport.h>

namespace EDS {

std::string EvManager::inv_key_str( "invalid key" );

// Remove references to remote objects, that was announced via 'channel'
// (related, may be, with socket connection)
// from [remote name -> local name] mapping table, and mark related session as
// 'disconnected'.
void EvManager::Remove( NetTransport *channel )
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
  disconnect( channel->sid() );
}

// return session id of object with address 'id' if this is external
// object; otherwise return -1;
EvSessionManager::key_type EvManager::sid( const key_type& id ) const
{
  MT_REENTRANT( _lock_heap, _1 );
  heap_type::const_iterator i = heap.find( id );
  if ( i == heap.end() || (*i).second.remote == 0 ) {
    return -1;
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

EvSessionManager::key_type EvManager::establish_session()
{
  // EvSessionManager::key_type new_key = smgr.create();

  return /* new_key */ smgr.create();
}

void EvManager::disconnect( const EvSessionManager::key_type& _sid )
{
  if ( smgr.is_avail( _sid ) ) {
    SessionInfo& info = smgr[_sid];
    info.disconnect();
//    cerr << "EvManager::disconnect: " << _sid << endl;
    if ( info._control != Event::badaddr ) {
      Event_base<Event::key_type> ev_disconnect( EV_DISCONNECT, _sid );
      ev_disconnect.dest( info._control );
//      cerr << "EvManager::disconnect, info._control: " << info._control << endl;
      Send( EDS::Event_convert<Event::key_type>()(ev_disconnect), Event::mgraddr );
//      cerr << "===== Pass" << endl;
    } else {
      smgr.erase( _sid );
    }
  }
}

} // namespace EDS

// -*- C++ -*- Time-stamp: <99/03/26 20:37:52 ptr>
#ident "%Z% $Date$ $Revision$ $RCSfile$ %Q%"

#include <EvManager.h>
#include <NetTransport.h>

namespace EDS {

std::string EvManager::inv_key_str( "invalid key" );

void EvManager::Remove( NetTransport *channel )
{
  MT_REENTRANT( _lock_heap, _1 );
  heap_type::iterator i = heap.begin();

  while ( i != heap.end() ) {
    if ( (*i).second.remote != 0 && (*i).second.remote->channel == channel ) {
      heap.erase( i++ );
    } else {
      ++i;
    }
  }
  disconnect( channel->sid() );
}

EvSessionManager::key_type EvManager::sid( const key_type& id ) const
{
  MT_REENTRANT( _lock_heap, _1 );
  heap_type::const_iterator i = heap.find( id );
  if ( i == heap.end() || (*i).second.remote == 0 ) {
    return -1;
  }
  return (*i).second.remote->channel->sid();
}

void EvManager::Dispatch( const Event& e )
{
  heap_type::iterator i = heap.find( e.dest() );
  if ( i != heap.end() && (*i).second.ref != 0 ) {
    (*i).second.ref->Dispatch( e );
  }
}

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
  const key_type _low  = 0x100;
  const key_type _high = /* 65535 */ 0x3fffffff;

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
    smgr[_sid].disconnect();
  }
}

} // namespace EDS

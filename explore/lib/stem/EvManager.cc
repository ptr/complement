// -*- C++ -*- Time-stamp: <99/03/19 17:29:31 ptr>
#ident "%Z% $Date$ $Revision$ $RCSfile$ %Q%"

#include <EvManager.h>
#include <NetTransport.h>

namespace EDS {

void EvManager::Send( const Event& e, const EvManager::key_type& src_id )
{
  dump( std::cerr, e );
  std::cerr << "Send: " << std::hex << src_id << "\n";
  heap_type::iterator i = heap.find( e.dest() );
  if ( i != heap.end() ) {
    if ( (*i).second.ref != 0 ) { // local deliver
       std::cerr << "Local\n";
      (*i).second.ref->Dispatch( e );
    } else { // remote object
       std::cerr << "Remote\n";
      __stl_assert( (*i).second.remote != 0 );
      (*i).second.remote->channel->push( e, (*i).second.remote->key, src_id );
    }
  } else {
    std::cerr << "Not found\n";
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

} // namespace EDS

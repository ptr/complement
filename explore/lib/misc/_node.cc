// -*- C++ -*- Time-stamp: <99/04/21 10:22:55 ptr>

#ident "%Z% $Date$ $Revision$ $RCSfile$ %Q%"

#include "node.h"

namespace Helios {

const std::string __nodes_heap_base::dot( "." );
const std::string __nodes_heap_base::dotdot( ".." );

 __nodes_heap_base::key_type __nodes_heap_base::create_unique()
{
  std::pair<heap_type::iterator,bool> ret;

  do {
    if ( ++_id > _high ) {
      _id = (_id - _low) % (_high - _low) + _low;
    }
  } while ( heap.find( _id ) != heap.end() );

  return _id;
}

} // namespace Helios

// -*- C++ -*- Time-stamp: <00/02/21 15:59:44 ptr>

/*
 *
 * Copyright (c) 1999
 * Petr Ovchenkov
 *
 * Copyright (c) 1999
 * ParallelGraphics Software Systems
 
 * This material is provided "as is", with absolutely no warranty expressed
 * or implied. Any use is at your own risk.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.
 */

#ident "$SunId$ %Q%"

#include <config/feature.h>

#ifndef _WIN32
#include "aux/node.h"
#else
#include "aux_/node.h"
#endif

namespace Helios {

const __STD::string __nodes_heap_base::dot( "." );
const __STD::string __nodes_heap_base::dotdot( ".." );

 __nodes_heap_base::key_type __nodes_heap_base::create_unique()
{
  __STD::pair<heap_type::iterator,bool> ret;

  do {
    if ( ++_id > _high ) {
      _id = (_id - _low) % (_high - _low) + _low;
    }
  } while ( heap.find( _id ) != heap.end() );

  return _id;
}

} // namespace Helios

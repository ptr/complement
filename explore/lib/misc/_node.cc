// -*- C++ -*- Time-stamp: <01/09/20 11:43:33 ptr>

/*
 *
 * Copyright (c) 1999
 * Petr Ovchenkov
 *
 * Copyright (c) 1999-2001
 * ParallelGraphics Ltd.
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
#pragma ident "@(#)$Id$"
#  endif
#endif

#include <config/feature.h>
#include "misc/node.h"

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
